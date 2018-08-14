#include <Wire.h>
#include "esperto_max30102.h"

MAX30102 particleSensor;

#define SENSOR_BUF_LENGTH 100 //data length
#define SPO2_BUF_LENGTH 10
uint16_t averageSpO2;

void setup()
{
  SerialUSB.begin(115200); // initialize serial communication at 115200 bits per second:

  // Initialize sensor
  particleSensor.begin(Wire, I2C_SPEED_STANDARD); //Use default I2C port, 400kHz speed

  SerialUSB.println(F("Attach sensor to finger with rubber band. Press any key to start conversion"));
  while (SerialUSB.available() == 0) ; //wait until user presses a key
  SerialUSB.read();

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A); // red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); // turn off Green LED
}

void loop()
{
    static uint32_t irBuffer[SENSOR_BUF_LENGTH]; //infrared LED sensor data
    static uint32_t redBuffer[SENSOR_BUF_LENGTH];  //red LED sensor data
    static int8_t senseBufIndex = 0; // used for circular buffer
    static int32_t spo2; //SPO2 value
    static int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
    static uint8_t spO2Buf[SPO2_BUF_LENGTH];
    static int8_t sp02BufIndex = 0; // used for circular buffer
    
    while (particleSensor.available() == false) //do we have new data?
      particleSensor.check(); //Check the sensor for new data

    redBuffer[senseBufIndex] = particleSensor.getRed();
    irBuffer[senseBufIndex] = particleSensor.getIR();
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
    senseBufIndex++;
    senseBufIndex%=SENSOR_BUF_LENGTH;

    //After gathering 25 new samples recalculate SP02
    if(senseBufIndex%25 == 0)
    {
        calcSpO2(irBuffer, SENSOR_BUF_LENGTH, redBuffer, &spo2, &validSPO2);
        
        if(validSPO2){
            spO2Buf[sp02BufIndex] = spo2;
            sp02BufIndex++;
            sp02BufIndex%=SPO2_BUF_LENGTH;
            averageSpO2 = 0;
            for(int i = 0; i < SPO2_BUF_LENGTH; i++){
              averageSpO2 += spO2Buf[i];
            }
            averageSpO2/=SPO2_BUF_LENGTH;
            SerialUSB.println(averageSpO2, DEC);
        }
    }
}

