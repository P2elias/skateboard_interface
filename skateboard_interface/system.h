#ifndef SYSTEM_H_INCLUDED
#define SYSTEM_H_INCLUDED

#include <stdint.h>

// pwm
void initPWM();

/** Sets the dutycycle of the pwm.
    Input:
    0 <= dutyCycle <= 100 every handed value > 100 will be interpreted as 100
 **/
void setPWMDutyCycle(uint8_t dutyCycle);

/** Changes the output channels for the pwm.
    Input:
    state = 0: A heavyside, C lowside
    state = 1: B heavyside, C lowside
    state = 2: B heavyside, A lowside
    state = 3: C heavyside, A lowside
    state = 4: C heavyside, B lowside
    state = 5: A heavyside, B lowside
    state > 5: power off all channels
**/
void changePhaseState(uint8_t state);
uint8_t getPhaseState();

// timers
void initTimers();

/** Calls the handed function after the handed time
Parameter:
time_ us    = time in us. max value: 2¹⁶ * 4 = 262'144us.
fn          = callback. Called after the handed time.
*/
void startAfterUs(uint32_t time_us, void (*fn)(void));

/** Starts a new time measurement.
resolution = 1/(16e6/64) = 4us
max. time = 4us * 2¹⁶ = 262.2ms

Input:
timerOverflowCallback: called after the max. time
*/
void startTimeMeasurement(void (*timerOverflowCallback)(void));

/** Returns a 1, if there is a running time measurement
*/
uint8_t isTimeMeasurementRunning();

/** Stops the time measurement and returns the measured time.
 return: measured time in us.
*/
uint32_t stopTimeMeasurement();

uint32_t getTime();

// analog values
void initAnalog();
int8_t startMeasureProcedure(char newPhaseToMeasure);
void registerMeasurementDataAvailableListener(void (*listener)(char phaseLastCurrentMeassure));

char readBatteryVoltage();
char readPhaseCurrnet(char phase); //value between 0 and 42, where 42 stands for 42 Ampére

// sensor 0 = Front
// sensor 1 = Back
uint8_t readInterfaceSensorsVoltage(char sensor);
uint8_t getLastHallSensorNoseVoltage();
uint8_t getLastHallSensorTailVoltage();
int8_t getLastPhaseACurrent();
int8_t getLastPhaseBCurrent();
uint8_t getLastBattery();

void readInterfaceSensorsVoltageBLOCKING(char sensor);

char readReference1BLOCKING();
char readReference2BLOCKING();
char readReference3BLOCKING();
char readReference4BLOCKING();

// comperators
void initComp();

/** Register the handed function as listener which is called when the voltage of the phase A crosses the zero
    Input:
    listener = function with parameter edge.
        --> edge = 0: falling edge
        --> edge = 1: rising edge
**/
void registerVoltageZeroCrossingListenerPhaseA(void (*listener)(char));
void registerVoltageZeroCrossingListenerPhaseB(void (*listener)(char));
void registerVoltageZeroCrossingListenerPhaseC(void (*listener)(char));

/** Enables the comperator for the phase A.
    Input:
    enable = 1: enable comperator
    enable = 0: disable comperator
**/
void setEnableCompA(char enable);
void setEnableCompB(char enable);
void setEnableCompC(char enable);

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
batteryPower = 1: Led 1 on
batteryPower = 2: Leds 1 & 2 on
batteryPower = 3: Leds 1,2 & 3 on
batteryPower > 3: All leds on
*/
void setLEDsBatteryPower(char batteryPower);

/** drive piezo sound element
state >= 1: turn piezo sound element on
state = 0: turn piezo sound element off
**/
void setPiezoSound(char state);

/** enable bridge driver
state >= 1: turn PWM on
state = 0: turn PWM off
**/
void enableBridgeDriver(char state);
void setDC_cal(uint8_t state);

/** Turns on Power LED
**/
void setPowerLED();

// systime
void initSystime();
uint16_t getSystimeMs();

// log
void initUART();
void transmitString(char *msg);
void transmitChar(char data);

// spi
void initSPI();
void spi_readStatusRegisters_BLOCKING();
uint16_t getLastStatusRegister1Value();
uint16_t getLastStatusRegister2Value();

#endif // SYSTEM_H_INCLUDED
