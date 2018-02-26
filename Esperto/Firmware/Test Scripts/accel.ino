 #include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"

// class default I2C address is 0x68
MPU6050 mpu;

#define INTERRUPT_PIN 2  // use pin 2 on Arduino Uno & most boards // digital I/O pin 2.

// MPU control/status vars
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container

// indicates whether MPU interrupt pin has gone high
volatile bool mpuInterrupt = false;
void dmpDataReady() {
    mpuInterrupt = true;
}

// Tynans vars        
const double stepDiffMinThreshold = 0.025; // minimum difference between step max and min for considered step
int stepCount = 0;                        // total number of steps taken
double stepMax = 0;                       // peak of quaternion data
double stepMin = 0;                       // trough of quaternion data
double currentQData[3];                   // array storing recent quaternion values

void setup() {
    Wire.begin();
    Serial.begin(9600);
    
    // initialize device
    mpu.initialize();
    pinMode(INTERRUPT_PIN, INPUT);

    // load and configure the DMP
    devStatus = mpu.dmpInitialize();

    // turn on the DMP, now that it's ready
    mpu.setDMPEnabled(true);

    // enable Arduino interrupt detection
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

//     get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
}

void countSteps()
{
  // get current MPU status
  mpuIntStatus = mpu.getIntStatus(); // get INT_STATUS byte
  fifoCount = mpu.getFIFOCount(); // get current FIFO count
  // otherwise, check for DMP data ready interrupt (this should happen frequently)
  if (mpuIntStatus & 0x02) 
  {
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount(); // wait for correct available data length, should be a VERY short wait
    mpu.getFIFOBytes(fifoBuffer, packetSize); // read a packet from FIFO
    // track FIFO count here in case there is > 1 packet available
    // (this lets us immediately read more without waiting for an interrupt)
    fifoCount -= packetSize;
    // get quaternion values
    mpu.dmpGetQuaternion(&q, fifoBuffer);
  }
  Serial.println(q.w*10);
  // update recent quaternion values
  currentQData[0] = currentQData[1];
  currentQData[1] = currentQData[2];
  currentQData[2] = q.w; 
  // if a peak/max is found
  if(currentQData[1] > currentQData[0] && currentQData[1] > currentQData[2])
  {
    stepMax = currentQData[1];// update peak value      
    double maxMinDiff = stepMax - stepMin; 
    // if a step is detected
    if (maxMinDiff > stepDiffMinThreshold)
    {
        stepCount++;
    }
  }
  // if a trough/min is found
  else if(currentQData[1] < currentQData[0] && currentQData[1] < currentQData[2])
  {
    stepMin = currentQData[1];
  }
}

void loop() {    
  // wait for MPU interrupt or extra packet(s) available
  if(mpuInterrupt || fifoCount > packetSize)
  {
    mpuInterrupt = false; // reset interrupt flag
    countSteps();
  }
}


