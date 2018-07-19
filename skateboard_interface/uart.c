/*
 * uart.c
 *
 * Created: 18.07.2018 11:24:37
 *  Author: elias.rotzler
 */ 

#include "system.h"

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/** UDRn – USART I/O Data Register n p.218
Wenn gelesen: Empfangene Daten von Receive Data Buffer Register (RXB)
Wenn beschrieben: Daten auf Data Buffer Register (TXB) schreiben
-> nur beschreibbar wenn UDREn Flag in UCSRAnA Register gesetzt (sonst ignoriert)*/
#define UART_SEND_DATA UDR0

/** UCSRnA – USART Control and Status Register A p.219
|7      |6      |5      |4      |3      |2      |1      |0      |
|RXCn   |TXCn   |UDREn  |FEn    |DORn   |UPEn   |U2Xn   |MPCMn  |
RXCn: USART Receive Complete
-> gesetzt wenn ungelesene Daten in receive register
TXCn: USART Transmit Complete
-> gesetzt wenn alle Daten gesendet & keine neuen daten in UDRn verfügbar
UDREn: USART Data Register Empty
-> gesetzt wenn UDRn leer ist
FEn: Frame Error
-> gesetzt wenn nächstes zeichen in receive buffer ein frame error aufweist
DORn: Data OverRun
-> gesetzt wenn empfangbuffer voll ist (zwei Zeichen)
UPEn: USART Parity Error
-> gesetzt wenn zeichen in eingansbuffer ein parity fehler aufweist
U2Xn: Double the USART Transmission Speed
-> wenn gesetzt, wird divisor von baudrate von 16 auf 8 reduziert --> verdoppelt speed
-> hat einfluss auf asynchrone übertragung (sonst auf 0 setzen!).
MPCMn: Multi-processor Communication Mode
-> Auf 0 setzen*/
#define UART_REGISTER_A UCSR0A
#define UART_DATA_REG_EMPTY_BIT UDRE0
#define UART_REGISTER_A_VALUE ((1<<U2X0) | (0<<MPCM0))
#define UART_RECIVE_COMPLETE RXC0

/** UCSRnB – USART Control and Status Register n B
|7      |6      |5      |4      |3      |2      |1      |0      |
|RXCIEn |TXCIEn |UDRIEn |RXENn  |TXENn  |UCSZn2 |RXB8n  |TXB8n  |
RXCIEn: RX Complete Interrupt Enable n
-> interrupt wenn RXCn gesetzt
TXCIEn: TX Complete Interrupt Enable n
-> interrupt wenn TXCn gesetzt
UDRIEn: USART Data Register Empty Interrupt Enable n
RXENn: Receiver Enable n
-> wenn gesetzt wird USART Empfänger aktiviert
TXENn: Transmitter Enable n
-> wenn gesetzt wird USART Sender aktiviert
UCSZn2: Character Size n
-> mit UCSZn1:0 aus UCSRnC. Legt anzahl von datenbits in einem Frame fest
RXB8n: Receive Data Bit 8 n
TXB8n: Transmit Data Bit 8 n
-> neuntes Datenbyt wenn mit 9 Bit gearbeitet wird*/
#define UART_REGISTER_B_VALUE ((0<<RXCIE0) | (0<<TXCIE0) | (0<<UDRIE0) | (0<<RXEN0) | (1<<TXEN0) | (1<<UCSZ02))
#define UART_REGISTER_B UCSR0B

/** UCSRnC – USART Control and Status Register n C p.221
|7      |6      |5      |4      |3      |2      |1      |0      |
|UMSELn1|UMSELn0|UPMn1  |UPMn0  |USBSn  |UCSZn1 |UCSZn0 |UCPOLn |
UMSELn1:0 USART Mode Select
-> Modus des USART:
0 0 async <-- keine gemeinsahme taktleitung!
0 1 sync
1 1 master SPI
UPMn1:0: Parity Mode
-> Festlegen des parity modus:
0 0 disabled <-- da nur zu debug-zwecken
1 0 enabled even
1 1 enabled odd
USBSn: Stop Bit Select
-> anzahl stop bits die sender einfügt
0 1-bit <-- normal
1 2-bit
UCSZn1:0: Character Size
-> anzahl datenbits (mit UCSZn2 aus UCSRnB)
0 1 0 7-bit
0 1 1 8-bit <-- default picocom
1 1 1 9-bit
UCPOLn: Clock Polarity
-> nur für sync mode. Beziehung zwischen daten ausgangs änderung, daten input sample und sync clock*/
#define UART_REGISTER_C_VALUE ((0<<UMSEL01) | (0<<UMSEL00) | (0<<UPM01) | (0<<UPM00) | (0<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00))
#define UART_REGISTER_C UCSR0C

/** UBRRnL and UBRRnH – USART Baud Rate Registers
|7      |6      |5      |4      |3      |2      |1      |0      |
|-      |-      |-      |-      |UBRR11 |UBRR10 |UBRR9  |UBRR8  | --> UBRRHn
|UBRR7  |UBRR6  |UBRR5  |UBRR4  |UBRR3  |UBRR2  |UBRR1  |UBRR0  | --> UBRRLn
Bit 15:12 -> reserviert, müssen mit 0 beschrieben werden
Bit 11:0 – UBRR11:0: USART Baud Rate Register
-> 12bit register für Baud rate
fehler muss unter 0.5% liegen
     |U2Xn = 0       |U2Xn = 1       |
Baud |UBRR   |Error  |UBRR   |Error  |
9600 |103    |0.2%   |207    |0.2%   | -> 103 = 0b0110 0111
57.6K|16     |2.1%   |32     |-0.8%  |
250K |3      |0.0%   |7      |0.0%   | -> 3   = 0b0000 0011
Table 22-12 p.226
103 --> 0b0110 0111*/
#define UART_BAUD_RATE_HIGH_BYTE UBRR0H
#define UART_BAUD_RATE_LOW_BYTE UBRR0L
#define UART_BAUD_RATE_HIGH_BYTE_VALUE 0
#define UART_BAUD_RATE_LOW_BYTE_VALUE 32;

/** Benutzung Linux
ttyACM3
Terminal starten:
sudo picocom --baud 9600  --parity n --databits 8 --flow n -l /dev/ttyACM3
(Quelle: https://developer.ridgerun.com/wiki/index.php/Setting_up_Picocom_-_Ubuntu)*/

/** Parameter:
Baud: 9600
Parity: No
Databits: 8
Stopbits: 1
*/

void initUART()
{
    UART_REGISTER_A |= UART_REGISTER_A_VALUE;
    UART_REGISTER_B |= UART_REGISTER_B_VALUE;
    UART_REGISTER_C |= UART_REGISTER_C_VALUE;

    UART_BAUD_RATE_HIGH_BYTE = UART_BAUD_RATE_HIGH_BYTE_VALUE;
    UART_BAUD_RATE_LOW_BYTE = UART_BAUD_RATE_LOW_BYTE_VALUE;
}

// Source: https://www.mikrocontroller.net/articles/AVR-GCC-Tutorial/Der_UART
void transmitString(char *msg)
{
    while (*msg) // point on first char
    {
        // msg is zero terminated -> loop will stop at the end of the string
        transmitChar(*msg);
        msg++; // increase pointer to next char
    }
}

// Source: Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet p.207
void transmitChar(char data)
{
    while(!(UART_REGISTER_A & (1<<UART_DATA_REG_EMPTY_BIT)));

    UART_SEND_DATA = data;
}

// Source: Atmel ATmega640/V-1280/V-1281/V-2560/V-2561/V Datasheet p.210
unsigned char receiveChar(void)
{
	while ( !(UART_REGISTER_A & (1<<UART_RECIVE_COMPLETE)) )
	;
	/* Get and return received data from buffer */
	return UART_SEND_DATA;
}