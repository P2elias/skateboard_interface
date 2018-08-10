/*
 * uart.c
 *
 * Created: 15.06.2018
 *  Author: elias.rotzler
 * Strucktur und kleine Teile aus dem P4 übernommen und auf das P6 angepasst
 * Kommentare auf Englisch stammen entweder aus dem Datenblatt zum Kontroller oder mehrheitlich aus dem P4
 * Kommentare auf Deutsch stammen aus dem P6
 */ 

#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include <stdint.h>

// gpios
void initGPIOs();

// adc
void initADC();
int readADC(int adc_channel);

// taster (Hauptschalter / Selbsthaltung Überbrückung)
char readTaster();

// Schaltet Eingangsspannung auf den Ausgang (zum Motorenkontroller) 
void enablePVDD2(char state);

// Gibt dem Motorenkontroller die Freigabe, den Betrieb aufzunehmen
void enableMotorController(char state);

// Schaltet Selbsthaltung ein
void enableSelbsthaltung(char state);

/** sets the 4 leds to visualize the battery power
batteryPower = 0: All leds off
batteryPower = 1: Led 1 toggle
batteryPower = 2: Led 1 on
batteryPower = 3: Leds 1 & 2 on
batteryPower = 4: Leds 1,2 & 3 on
batteryPower > 4: All leds on
*/
void setLEDsBatteryPower(char batteryPower);

/** drive piezo sound element
state >= 1: turn piezo sound element on
state = 0: turn piezo sound element off
**/
void setPiezoSound(char state);

/** Turns on Power LED
**/
void setPowerLED();

// uart initialisierung
void initUART();

// empfangen über uart
unsigned char receiveChar(void);

// char senden über uart (eine Übertragung)
void transmitChar(char data);

// char-Folge senden über uart
void transmitString(char *msg);
#endif // SYSTEM_H_INCLUDED