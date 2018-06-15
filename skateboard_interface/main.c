/*
 * skateboard_interface.c
 *
 * Created: 15.06.2018 14:11:05
 * Author : elias.rotzler
 */ 

#include <avr/io.h>
#include "system.h"


int main(void)
{
	initGPIOs();
	setLEDsBatteryPower(4);
	
    while (1) 
    {
    }
}

