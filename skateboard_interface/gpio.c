#include <avr/io.h>
//#include "logger.h"

#define TRISTATE_INPUT 0
#define TRISTATE_OUTPUT 1
#define PORT_PULLUP 1

#define PORT_POWER_LED PORTA
#define TRISTATE_POWER_LED DDRA
#define PIN_POWER_LED 0

#define PORT_LEDS PORTA
#define TRISTATE_LEDS DDRA
#define PIN_LEDs PINA
#define PIN_LED2 1
#define PIN_LED3 2
#define PIN_LED4 3
#define PIN_LED5 4

#define PORT_PIEZO PORTA
#define TRISTATE_PIEZO DDRA
#define PIN_PIEZO_PIN PINA
#define PIN_PIEZO 5

// Wird verwendet um die LED2 blinken und den PIEZZO ertönen zu lassen 
#define nTIMES 10
static int countToToggle = nTIMES;
//static int countToToggle2 = nTIMES;

#define PORT_EN_CNTRLLR PORTG
#define TRISTATE_EN_CNTRLLR DDRG
#define PIN_EN_CNTRLLR 4

#define PORT_VBAT_PVDD2_SCHALTER PORTL
#define TRISTATE_VBAT_PVDD2_SCHALTER DDRL
#define PIN_VBAT_PVDD2_SCHALTER 1

#define PORT_TASTER PORTL
#define TRISTATE_TASTER DDRL
#define PIN_TASTER 2
#define READ_TASTER_PIN PINL

#define PORT_SELBSTHALT_SCHALTER PORTL
#define TRISTATE_SELBSTHALT_SCHALTER DDRL
#define PIN_SELBSTHALT_SCHALTER 3

#define PORT_SOLL_MOMENT PORTK
#define TRISTATE_SOLL_MOMENT DDRK
#define PIN_SOLL_MOMENT 0

#define PORT_HALL_NOSE PORTK
#define TRISTATE_HALL_NOSE DDRK
#define PIN_HALL_NOSE 1

#define PORT_HALL_TAIL PORTK
#define TRISTATE_HALL_TAIL DDRK
#define PIN_HALL_TAIL 2

#define PORT_PVDD_SENSE PORTK
#define TRISTATE_PVDD_SENSE DDRK
#define PIN_PVDD_SENSE 4

#define PORT_ABGLEICH PORTF
#define TRISTATE_ABGLEICH DDRF
#define PIN_ABGLEICH_FAHRER_GEWICHT 1
#define PIN_ABGLEICH_BAT_MIN 2
#define PIN_ABGLEICH_BAT_MAX 3
#define PIN_ABGLEICH_TAIL_NEUTRAL 4
#define PIN_ABGLEICH_TAIL_MAX 5
#define PIN_ABGLEICH_NOSE_NEUTRAL 6
#define PIN_ABGLEICH_NOSE_MAX 7

//#define PORT_BRIDGE_DRIVER PORTJ
//#define TRISTATE_BRIDGE_DRIVER DDRJ
//#define PIN_EN_GATE 0
//#define PIN_PWRGD 4
//#define PIN_nOCTW 3
//#define PIN_nFAULT 2
//#define PIN_DC_CAL 1

void initGPIOs()
{
    //logMsgLn("Init GPIOs...");
    TRISTATE_LEDS |= ((TRISTATE_OUTPUT<<PIN_LED2) | (TRISTATE_OUTPUT<<PIN_LED3) | (TRISTATE_OUTPUT<<PIN_LED4) | (TRISTATE_OUTPUT<<PIN_LED5));
    TRISTATE_POWER_LED |= (TRISTATE_OUTPUT<<PIN_POWER_LED);
    TRISTATE_PIEZO |= (TRISTATE_OUTPUT<<PIN_PIEZO);
    TRISTATE_EN_CNTRLLR |= (TRISTATE_OUTPUT<<PIN_EN_CNTRLLR);
	TRISTATE_VBAT_PVDD2_SCHALTER |= ((TRISTATE_OUTPUT<<PIN_VBAT_PVDD2_SCHALTER) | (TRISTATE_OUTPUT<<PIN_TASTER) | (TRISTATE_OUTPUT<<PIN_SELBSTHALT_SCHALTER));
	
	TRISTATE_SOLL_MOMENT |= ((TRISTATE_INPUT<<PIN_SOLL_MOMENT) | (TRISTATE_INPUT<<PIN_HALL_NOSE) | (TRISTATE_INPUT<<PIN_HALL_TAIL) | (TRISTATE_INPUT<<PIN_PVDD_SENSE));
	TRISTATE_ABGLEICH |= ((TRISTATE_INPUT<<PIN_ABGLEICH_FAHRER_GEWICHT) | (TRISTATE_INPUT<<PIN_ABGLEICH_BAT_MIN) | (TRISTATE_INPUT<<PIN_ABGLEICH_BAT_MAX) | (TRISTATE_INPUT<<PIN_ABGLEICH_TAIL_NEUTRAL) | (TRISTATE_INPUT<<PIN_ABGLEICH_TAIL_MAX) | (TRISTATE_INPUT<<PIN_ABGLEICH_NOSE_NEUTRAL) | (TRISTATE_INPUT<<PIN_ABGLEICH_NOSE_MAX));
}

void initADC(){
	// select VCC as ADC Reference
	ADMUX = (1 << REFS0);
	// set prescaler for F_CPU = 16MHz (according Datasheet p.271 : 50kHz <= ADC_Clock <= 200kHz)
	// 16MHz/50kHz < ADC_PSC < 16MHz/200kHz  --->  320 < PSC < 80  --> PSC = 128 --> ADCCLK = 125kHz
	ADCSRA = (1<<ADEN) | (0b111<<ADPS0);
}

/** Turns on Power LED
**/
void setPowerLED()
{
	PORT_POWER_LED |= (1<<PIN_POWER_LED);
}

/** Drive piezo sound element
state >= 1: turn piezo sound element on
state = 0: turn piezo sound element off
**/
void setPiezoSound(char state)
{
	if(state)
	{
		PORT_PIEZO |= (1<<PIN_PIEZO);
	}
	else
	{
		PORT_PIEZO &= ~(1<<PIN_PIEZO);
	}
}

/** sets the 4 leds to visualize the battery power
batteryPower = 0: All leds off
batteryPower = 1: Led 1 on
batteryPower = 2: Leds 1 & 2 on
batteryPower = 3: Leds 1,2 & 3 on
batteryPower > 3: All leds on
*/
void setLEDsBatteryPower(char batteryPower)
{	
    if(batteryPower>3)
    {
        PORT_LEDS |= (1<<PIN_LED5);
    }
    else
    {
        PORT_LEDS &= ~(1<<PIN_LED5);
    }

    if(batteryPower>2)
    {
        PORT_LEDS |= (1<<PIN_LED4);
    }
    else
    {
        PORT_LEDS &= ~(1<<PIN_LED4);
    }

    if(batteryPower>1)
    {
        PORT_LEDS |= (1<<PIN_LED3);
    }
    else
    {
        PORT_LEDS &= ~(1<<PIN_LED3);
    }

    if(batteryPower>0)
    {
        PORT_LEDS |= (1<<PIN_LED2);
		PORT_PIEZO &= ~(1<<PIN_PIEZO);
    }
    else
    {
		if (countToToggle)
		{
			countToToggle -= 1;
			//PORT_LEDS |= (1<<PIN_LED2);
			//setPiezoSound(1);
		}
		else
		{
			PIN_LEDs |= (1<<PIN_LED2);
			PIN_PIEZO_PIN |= (1<<PIN_PIEZO);
			countToToggle = nTIMES;
			/*if (countToToggle2)
			{
				countToToggle2 -= 1;
				PORT_LEDS &= ~(1<<PIN_LED2);
				setPiezoSound(0);
			}
			else
			{
				countToToggle = nTIMES;
				countToToggle2 = nTIMES;
			}*/
			
		}
        
    }
}

/** enable Selbsthaltung
state >= 1: turn SELBSTHALTUNG_SCHALTER on
state = 0: turn SELBSTHALTUNG_SCHALTER off
**/
void enableSelbsthaltung(char state)
{
	
	if(state)
	{
		PORT_SELBSTHALT_SCHALTER |= (1<<PIN_SELBSTHALT_SCHALTER);
	}
	else
	{
		PORT_SELBSTHALT_SCHALTER &= ~(1<<PIN_SELBSTHALT_SCHALTER);
	}
}

/** enable Motor-controller (CNTRLLR)
state >= 1: turn PWM on
state = 0: turn PWM off
**/
void enableMotorController(char state)
{
    if(state)
    {
        PORT_EN_CNTRLLR |= (1<<PIN_EN_CNTRLLR);
    }
    else
    {
        PORT_EN_CNTRLLR &= ~(1<<PIN_EN_CNTRLLR);
    }
}

/** enable Power to Motor-controller
state >= 1: turn VBAT+_PVDD2_SCHALTER on
state = 0: turn VBAT+_PVDD2_SCHALTER off
**/
void enablePVDD2(char state)
{
	if(state)
	{
		PORT_VBAT_PVDD2_SCHALTER |= (1<<PIN_VBAT_PVDD2_SCHALTER);
	}
	else
	{
		PORT_VBAT_PVDD2_SCHALTER &= ~(1<<PIN_VBAT_PVDD2_SCHALTER);
	}
}

/*void setDC_cal(uint8_t state)
{
    if(state)
    {
        PORT_BRIDGE_DRIVER |= (1<<PIN_DC_CAL);
    }
    else
    {
        PORT_BRIDGE_DRIVER &= ~(1<<PIN_DC_CAL);
    }
}*/

/** Taster auslesen (Hauptschalter), (Selbsthaltung Überbrückung)
state >= 1: turn selbsthaltung on
state = 0: turn selbsthaltung off
**/
char readTaster()
{
	char state;
	state = (READ_TASTER_PIN & (1<<PIN_TASTER));
	return state;
}

int readADC(int adc_channel){
	ADMUX = (1 << REFS0);
	ADCSRB = 0;
	
	switch(adc_channel)
	{
		case 0:
		break;
		
		case 1:
			ADMUX |= (1 << MUX0);
		break;
		
		case 2:
			ADMUX |= (1 << MUX1);
		break;
		
		case 3:
			ADMUX |= (0b11 << MUX0);
		break;
		
		case 4:
			ADMUX |= (1 << MUX2);
		break;
		
		case 5:
			ADMUX |= (0b101 << MUX0);
		break;
		
		case 6:
			ADMUX |= (0b11 << MUX1);
		break;
		
		case 7:
			ADMUX |= (0b111 << MUX0);
			
		break;
		
		case 8:
			ADCSRB |= (1 << MUX5);
		break;
		
		case 9:
			ADCSRB |= (1 << MUX5);
			ADMUX |= (1 << MUX0);
		break;
		
		case 10:
			ADCSRB |= (1 << MUX5);
			ADMUX |= (1 << MUX1);
		break;
		
		case 11:
			ADCSRB |= (1 << MUX5);
			ADMUX |= (0b11 << MUX0);
		break;
		
		case 12:
			ADCSRB |= (1 << MUX5);
			ADMUX |= (1 << MUX2);
		break;
		
		case 13:
			ADCSRB |= (1 << MUX5);
			ADMUX |= (0b101 << MUX0);
		break;
		
		case 14:
			ADCSRB |= (1 << MUX5);
			ADMUX |= (0b11 << MUX1);
		break;
		
		case 15:
			ADCSRB |= (1 << MUX5);
			ADMUX |= (0b111 << MUX0);
		break;
	}
	
	// trigger ADC by setting ADSC flag 	(setting of ADIF would clear the ADIF flag as well - unneeded here)
	ADCSRA |= (1 << ADSC);
	// wait till ADC conversion is finished ...
	while (ADCSRA & (1 << ADSC)) {
		/* warten */
	}
	return ADC;
}


