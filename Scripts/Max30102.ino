#include <SAMD_AnalogCorrection.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

// Create instance of MAX30105 class
MAX30105 heartRateSensor;

const byte arraySizeHR = 5; // size of array containing latest HR values
byte heartRates[arraySizeHR]; // array containing latest HR values
byte heartRateIndex = 0;
long prevHeartBeat = 0; // time at which the last heart beat occurred
float heartRate; // current heart rate
int beatAvg; // average heart rate which will we be displayed

void setup()
{
  SerialUSB.begin(9600);

  // Turn on and setup heart rate sensor
  heartRateSensor.begin(Wire, I2C_SPEED_STANDARD);
  heartRateSensor.setup(); //Configure sensor with default settings
  heartRateSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  heartRateSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void loop()
{
  // obtain infrared value
  long irValue = heartRateSensor.getIR();

  // if heart beat was detected
  if (checkForBeat(irValue) == true)
  {
    // calculate time difference between 2 beats
    long heartBeatTimeDiff = millis() - prevHeartBeat;
    prevHeartBeat = millis();

    // use the difference to calculate the heart rate
    heartRate = 60 / (heartBeatTimeDiff / 1000.0); // 60 s in 1 min, 1000 ms in 1 s

    // only use valid heart rates
    if (heartRate < 120 && heartRate > 40)
    {
      heartRates[heartRateIndex++] = (byte)heartRate; // store heart rate in array
      heartRateIndex %= arraySizeHR; // use modulus op. to determine current index

      // calcute average heart rate
      beatAvg = 0; // reset
      for (int i = 0; i < arraySizeHR; i++)
        beatAvg += heartRates[i]; // add up all heart rates
      beatAvg /= arraySizeHR; // determine average by dividing
    }
  }

  SerialUSB.print("IR=");
  SerialUSB.print(irValue);
  SerialUSB.print(", BPM=");
  SerialUSB.print(heartRate);
  SerialUSB.print(", Avg BPM=");
  SerialUSB.println(beatAvg);
}
