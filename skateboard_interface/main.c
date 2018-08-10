/*
 * skateboard_interface.c
 *
 * Created: 15.06.2018 14:11:05
 * Author : elias.rotzler
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include "system.h"

// LED Status
#define FOUR_LEDS 5
#define THREE_LEDS 4
#define TWO_LEDS 3
#define ONE_LED 2
#define ONE_LED_TOGGLE 1
#define LEDS_OFF 0

// Akku-Niveau
#define UEBERSPANNUNG 931
#define	AKKU_GELADEN 800
#define AKKU_OBEREMITTE 600
#define AKKU_UNTEREMITTE 400
#define AKKU_LEER 133
#define UNTERSPANNUNG 50

// Offset-Bereinigung
/* Quellenspannungsüberwachung	mit U_adc_max = 26 V * 24/124 = 5.0 V und U_adc_min = 18 V * 24/124 = 3.5 V
								entspricht:						1024								  717
								gemessen:						4.67 V								  3.48
								entspricht:						956 soll aber 1024 sein				  713 soll aber 0 sein, daher:
								1024/(956-713) = 4.21
Funktion:	4.21 * (PVDDsense-717)
*/
#define PVDD_SENSE_OFFSET 713
#define PVDD_SENSE_FACTOR 4.21

// ADC-Nummer
#define ADC_RESERVE 1
#define	BAT_MIN 2
#define BAT_MAX 3
#define TAIL_NEUTRAL 4
#define TAIL_MAX 5
#define NOSE_NEUTRAL 6
#define NOSE_MAX 7
#define SOLLMOMENT_POTI 8
#define HALL_NOSE 9
#define HALL_TAIL 10
#define PVDD_SENSE 12

// // für uart tests:
// #define kTIMES 100
// static int countToToggle = kTIMES;
// static int countToToggle2 = kTIMES;
// #define PORT_LED PORTD
// #define TRISTATE_LED DDRD
// #define PIN_LED_D PIND
// #define PIN_LED 0

int main(void)
{
 	// für uart Test:
// 	TRISTATE_LED |= (1<<PIN_LED);
// 	PORT_LED |= (1<<PIN_LED);
	
	// Variable für den Zustand der Selbsthaltung
	int volatile selbsthaltung;
	
	// Auswahl variablen (solten durch physiche Jumper realisiert werden)
	int sollMomentWahl = 1;	// sollMomentWahl >= 1: Sollvorgabe über Hallsensoren im Skateboard
							// sollMomentWahl = 0: Sollmomentvorgabe über Poti
	int abgleichEnable = 0;	// abgleichEnable = 0: Abgleich nicht erlaubt
							// abgleichEnable >= 1: Abgleich erlaubt
	
	// adc Variablen
	int sollMomentPoti, hallNose, hallTail, PVDDsense;
	// adc Variablen die erst verwendet werden, wenn der Abgleich erlaubt ist
//	int adcReserve, batMin, batMax, tailNeutral, tailMax, noseNeutral, noseMax;
	
	// Hilfsvariaben, Rechengrössen
 	int32_t sollMoment;
 	int uQuelle;
// 	char uartTest;		// für den Test der uart Übertragung
	
	//initUART();
	initGPIOs();
	initADC();
		
	while (1) 
    {
		if (!selbsthaltung)
		{
			selbsthaltung = readTaster();
			if (selbsthaltung) // erlaubt das Einschalten nur wenn Selbsthaltung aktiv ist
			{
				setPowerLED();
				enableSelbsthaltung(1);
				//enableMotorController(1);		// nur solange Batteriemessung auskomentiert, sonst löschen
				//enablePVDD2(1);				// nur solange Batteriemessung auskomentiert, sonst löschen
				
				
				// einlesen der Abgleichpotis wird nicht verwendet, da deren Funktion in der restlichen Software noch nicht eingebunden ist
// 				fahrerGewicht = readADC(FAHRER_GEWICHT);
// 				batMin = readADC(BAT_MIN);
// 				batMax = readADC(BAT_MAX);
// 				tailNeutral = readADC(TAIL_NEUTRAL);
// 				tailMax = readADC(TAIL_MAX);
// 				noseNeutral = readADC(NOSE_NEUTRAL);
// 				noseMax = readADC(NOSE_MAX);
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
			PVDDsense = readADC(PVDD_SENSE);
			uQuelle = PVDD_SENSE_FACTOR * (PVDDsense - PVDD_SENSE_OFFSET);			
				if(uQuelle>UEBERSPANNUNG)	// weiteres laden des Akkus würde zu Überspannung führen (Zerstörungsgefahr) daher Abschalten
				{
					setLEDsBatteryPower(FOUR_LEDS);
					setPiezoSound(1);
					enableMotorController(0);
					enablePVDD2(0);
					selbsthaltung = 0;
					enableSelbsthaltung(0);
				}
				else if (uQuelle>AKKU_GELADEN) // Akku ist voll geladen
				{
					setLEDsBatteryPower(FOUR_LEDS);
					setPiezoSound(0);
					enableMotorController(1);
					enablePVDD2(1);
				}
				else if(uQuelle>AKKU_OBEREMITTE)
				{
					setLEDsBatteryPower(THREE_LEDS);
					setPiezoSound(0);
					enableMotorController(1);
					enablePVDD2(1);
				}
				else if(uQuelle>AKKU_UNTEREMITTE)
				{
					setLEDsBatteryPower(TWO_LEDS);
					setPiezoSound(0);
					enableMotorController(1);
					enablePVDD2(1);
				}
				else if(uQuelle>AKKU_LEER)
				{
					setLEDsBatteryPower(ONE_LED);
					setPiezoSound(0);
					enableMotorController(1);
					enablePVDD2(1);
				}
				else if(uQuelle>UNTERSPANNUNG)	// Ist der Akku sogut wie leer, aber noch nicht unterspannungsgefährdet, Blinkt die LED, der Piezo gibt ein Warnsignal ab und der Motorkontroller wird abgeschaltet
				{
					setLEDsBatteryPower(ONE_LED_TOGGLE);
					enableMotorController(0);
					enablePVDD2(0);
					setPiezoSound(1);
				}
				else				// weinteres Entladen würde zu Unterspannung führen (Zerstörungsgefahr), daher Abschalten der LEDs, Piezo-Summer, Motorkontrollers und schliesslich der Selbsthaltung
				{
					setLEDsBatteryPower(LEDS_OFF);
					setPiezoSound(0);
					enableMotorController(0);
					enablePVDD2(0);
					selbsthaltung = 0;
					enableSelbsthaltung(0);	
				}
						
			// falls der Abgleich erlaubt ist (jumper müsste noch gelayoutet und hinzugefügt werden), werden die Abgleichpotis ausgelesen
			if (abgleichEnable)		// Die Variable abgleichEnable müsste durch das auslesen eines diegitalen Inputs ersetzt werden
			{
				// das Auslesen wird erst benötigt, wenn der Abgleich im restlichen Programm implementiert ist
// 				fahrerGewicht = readADC(ADC_RESERVE);
// 				batMin = readADC(BAT_MIN);
// 				batMax = readADC(BAT_MAX);
// 				tailNeutral = readADC(TAIL_NEUTRAL);
// 				tailMax = readADC(TAIL_MAX);
// 				noseNeutral = readADC(NOSE_NEUTRAL);
// 				noseMax = readADC(NOSE_MAX);
			}
												
			/** Wahl der Quelle für die Sollmomentvorgabe
			sollMomentWahl >= 1: Sollvorgabe über Hallsensoren im Skateboard
			sollMomentWahl = 0: Sollmomentvorgabe über Poti
			**/
			if (sollMomentWahl)
			{
				hallNose = readADC(HALL_NOSE);
				hallTail = readADC(HALL_TAIL);
				sollMoment = hallNose/2 - hallTail/2;
			}
			else
			{
				sollMomentPoti = readADC(SOLLMOMENT_POTI);
				sollMoment = sollMomentPoti/2 - 256;				
			}
		}
    }
}



// UART
//initUART();

/*			//UART-TEST
			// 1 für sendenden Kontroller, 0 für empfangenden; es muss auch in uart.c UART_BAUD_RATE_LOW_BYTE_VALUE angepasst werden
			if (0)
			{
				uartTest = (char) (readADC(8)>>2);
				if (countToToggle)
				{
					countToToggle -= 1;
					if (countToToggle == 2)
					{
						transmitChar(uartTest);
					}
					
				}
				else
				{
					PIND |= (1<<PIN_LED);
					countToToggle = kTIMES;
				}
			}

			else
			{
				char recived = receiveChar();
				
// 				if (countToToggle)
// 				{
// 					countToToggle -= 1;
// 				}
// 				else
// 				{
// 					PINA |= (1<<3);
// 					countToToggle = nTIMES;
// 				}
				
				if(recived>250)
				{
					setLEDsBatteryPower(4);
				}
				else if(recived>150)
				{
					setLEDsBatteryPower(3);
				}
				else if(recived>100)
				{
					setLEDsBatteryPower(2);
				}
				else if(recived>50)
				{
					setLEDsBatteryPower(1);
				}
				else
				{
					setLEDsBatteryPower(0);
				}
			}*/

// Sollmoment:
			// LEDs nutzen um Sollmoment darzustellen			
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
				setLEDsBatteryPower(4);*/
