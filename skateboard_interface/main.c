/*
 * skateboard_interface.c
 *
 * Created: 15.06.2018 14:11:05
 * Author : elias.rotzler
 */ 

//#define F_CPU 16000000UL	// 16 MHz
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "system.h"

/*#define FOSC 16000000// Clock Speed
#define BAUD 9600
#define MYUBRR FOSC/16/BAUD-1

void USART_Init( unsigned int ubrr){
	// Set baud rate
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	// Enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	// Set frame format: 8data, 1stop bit
	UCSR0C = (0b11<<UCSZ00);
} // USART_Init

unsigned char USART_Receive( void )
{
	// Wait for data to be received
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	// Get and return received data from buffer
	return UDR0;
}

void USART_Transmit( unsigned char data )
{
	// Wait for empty transmit buffer
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	// Put data into buffer, sends the data
	UDR0 = data;
}*/

int main(void)
{
	// Variable für den Zustand der Selbsthaltung
	int volatile selbsthaltung, selbsthaltError;
	
	// Auswahl variablen (solten durch physiche Jumper realisiert werden)
	int sollMomentWahl = 1, abgleichEnable = 1;
	
	// adc Variablen
	int adc, fahrerGewicht, batMin, batMax, tailNeutral, tailMax, noseNeutral, noseMax, sollMomentPoti, hallNose, hallTail, PVDDsense;
	
	// Hilfsvariaben, Rechengrössen
	int32_t sollMoment;
	int uQuelle;
	
	//USART_Init (MYUBRR);
	
	initGPIOs();
	initADC();
	
	while (1) 
    {
		if (!selbsthaltung)
		{
			selbsthaltung = readTaster();
			if (selbsthaltung && (readADC(12) > 760))
			{
				setPowerLED();
				enableSelbsthaltung(1);
			}
			else
			{
				enableMotorController(0);
				enablePVDD2(0);
				enableSelbsthaltung(0);
				
			}
		}
		else
		{
			
			/* Quellenspannungsüberwachung	mit U_adc_max = 26 V * 24/124 = 5.0 V und U_adc_min = 18 V * 24/124 = 3.5 V
											entspricht:						1024								  717 soll aber 0 sein, daher:
											1024/(1024-717) = 3.34
				Funktion:	3.34 * (PVDDsense-717)
			*/
			
			PVDDsense = readADC(12);
			uQuelle = 3.34 * (PVDDsense - 717);
				if(uQuelle>900)
				{
					//enableSelbsthaltung(1);
					setLEDsBatteryPower(4);
					enableMotorController(1);
					enablePVDD2(1);
				}
				else if(uQuelle>600)
				{
					//enableSelbsthaltung(1);
					setLEDsBatteryPower(3);
					enableMotorController(1);
					enablePVDD2(1);
				}
				else if(uQuelle>350)
				{
					//enableSelbsthaltung(1);
					setLEDsBatteryPower(2);
					enableMotorController(1);
					enablePVDD2(1);
				}
				else if(uQuelle>150)
				{
					//enableSelbsthaltung(1);
					setLEDsBatteryPower(1);
					enableMotorController(1);
					enablePVDD2(1);
				}
				else if(uQuelle>40)
				{
					//enableSelbsthaltung(1);
					setLEDsBatteryPower(0);
					enableMotorController(0);
					enablePVDD2(0);
				}
				else
				{
					setLEDsBatteryPower(0);
					enableMotorController(0);
					enablePVDD2(0);
					selbsthaltung = 0;
					enableSelbsthaltung(0);	
				}
						
			// falls der Abgleich erlaubt ist (jumper muss noch gelayoutet und hinzugefügt werden) 
			if (abgleichEnable)
			{
				fahrerGewicht = readADC(1);
				batMin = readADC(2);
				batMax = readADC(3);
				tailNeutral = readADC(4);
				tailMax = readADC(5);
				noseNeutral = readADC(6);
				noseMax = readADC(7);
			}
						
			sollMomentPoti = readADC(8);
			hallNose = readADC(9);
			hallTail = readADC(10);
			
			//USART_Transmit(USART_Receive());
												
			/** Wahl der Quelle für die Sollmomentvorgabe
			sollMomentWahl >= 1: Sollvorgabe über Hallsensoren im Skateboard
			sollMomentWahl = 0: Sollmomentvorgabe über Poti
			**/
			if (sollMomentWahl)
			{
				sollMoment = hallNose/2 - hallTail/2;
			}
			else
			{
				sollMoment = sollMomentPoti/2 - 256;				
			}
						
/*			if (sollMoment >= 200)
				setLEDsBatteryPower(4);
			if ((sollMoment < 200) & (sollMoment >= 100))
				setLEDsBatteryPower(3);
			else if((sollMoment < 100) & (sollMoment >= 0))
				setLEDsBatteryPower(2);
			else if((sollMoment < 0) & (sollMoment >= -100))
				setLEDsBatteryPower(1);
			else if((sollMoment < -100) & (sollMoment >= -200))
				setLEDsBatteryPower(0);
			else if(sollMoment < -200)
				setLEDsBatteryPower(4);
*/
			if (selbsthaltError)
			{
				// selbsthaltung = 0;
			}
		}
    }
}

