#include <avr/io.h>
//#include "logger.h"

#define TRISTATE_INPUT 0
#define TRISTATE_OUTPUT 1

#define PORT_LEDS PORTA
#define TRISTATE_LEDS DDRA
#define PIN_LED0 1
#define PIN_LED1 2
#define PIN_LED2 3
#define PIN_LED3 4

#define PORT_POWER_LED PORTA
#define TRISTATE_POWER_LED DDRA
#define PIN_POWER_LED 0

#define PORT_PIEZO PORTA
#define TRISTATE_PIEZO DDRA
#define PIN_PIEZO 5

#define PORT_BRIDGE_DRIVER PORTJ
#define TRISTATE_BRIDGE_DRIVER DDRJ
#define PIN_EN_GATE 0
#define PIN_PWRGD 4
#define PIN_nOCTW 3
#define PIN_nFAULT 2
#define PIN_DC_CAL 1

void initGPIOs()
{
    //logMsgLn("Init GPIOs...");

    TRISTATE_LEDS |= ((TRISTATE_OUTPUT<<PIN_LED0) | (TRISTATE_OUTPUT<<PIN_LED1) | (TRISTATE_OUTPUT<<PIN_LED2) | (TRISTATE_OUTPUT<<PIN_LED3));
    TRISTATE_POWER_LED |= (TRISTATE_OUTPUT<<PIN_POWER_LED);
    TRISTATE_PIEZO |= (TRISTATE_OUTPUT<<PIN_PIEZO);
    TRISTATE_BRIDGE_DRIVER |= ((TRISTATE_OUTPUT<<PIN_EN_GATE) | (TRISTATE_INPUT<<PIN_PWRGD) | (TRISTATE_INPUT<<PIN_nFAULT) | (TRISTATE_INPUT<<PIN_DC_CAL));
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
        PORT_LEDS |= (1<<PIN_LED3);
    }
    else
    {
        PORT_LEDS &= ~(1<<PIN_LED3);
    }

    if(batteryPower>2)
    {
        PORT_LEDS |= (1<<PIN_LED2);
    }
    else
    {
        PORT_LEDS &= ~(1<<PIN_LED2);
    }

    if(batteryPower>1)
    {
        PORT_LEDS |= (1<<PIN_LED1);
    }
    else
    {
        PORT_LEDS &= ~(1<<PIN_LED1);
    }

    if(batteryPower>0)
    {
        PORT_LEDS |= (1<<PIN_LED0);
    }
    else
    {
        PORT_LEDS &= ~(1<<PIN_LED0);
    }
}

/** enable bridge driver
state >= 1: turn PWM on
state = 0: turn PWM off
**/
void enableBridgeDriver(char state)
{
    if(state)
    {
        PORT_BRIDGE_DRIVER |= (1<<PIN_EN_GATE);
    }
    else
    {
        PORT_BRIDGE_DRIVER &= ~(1<<PIN_EN_GATE);
    }
}

void setDC_cal(uint8_t state)
{
    if(state)
    {
        PORT_BRIDGE_DRIVER |= (1<<PIN_DC_CAL);
    }
    else
    {
        PORT_BRIDGE_DRIVER &= ~(1<<PIN_DC_CAL);
    }
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
