#include <SAMD_AnalogCorrection.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include <RTCZero.h>
#include <SD.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <SparkFunBLEMate2.h>

extern "C" char *sbrk(int i);

// Bootloader logo: Width x Height = 64,64
static const uint8_t boot[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
  0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xf8, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc,
  0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0xc0, 0xff, 0xfd, 0x03, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff,
  0xef, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x3f, 0x00, 0x00,
  0x00, 0x00, 0xfe, 0xff, 0x7f, 0x5b, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
  0xef, 0xff, 0x01, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0xf6, 0x07, 0x00,
  0x00, 0xe0, 0xff, 0xff, 0xec, 0xdf, 0x06, 0x00, 0x00, 0xf0, 0xff, 0x3f,
  0xf8, 0xf7, 0x01, 0x00, 0x00, 0xe0, 0xff, 0x1f, 0xd0, 0x7e, 0x01, 0x00,
  0x00, 0xe0, 0xff, 0x07, 0xc0, 0x17, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x01,
  0x00, 0x1e, 0x00, 0x00, 0x00, 0xf0, 0x7f, 0x00, 0x00, 0x02, 0x00, 0x00,
  0x00, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xf0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xe0, 0x7f, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0xf0, 0x7f, 0x00,
  0x80, 0x3f, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0xe0, 0x7f, 0x00, 0x00,
  0x00, 0xe0, 0x7f, 0x00, 0xf8, 0xff, 0x01, 0x00, 0x00, 0xe0, 0x7f, 0x00,
  0xfc, 0xff, 0x07, 0x00, 0x00, 0xf0, 0x7f, 0x00, 0xfe, 0xff, 0x07, 0x00,
  0x00, 0xe0, 0x7f, 0x00, 0xfc, 0xff, 0x07, 0x00, 0x00, 0xe0, 0x7f, 0x00,
  0xf0, 0xff, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0xc0, 0x7f, 0x00, 0x00,
  0x00, 0xf0, 0x7f, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00,
  0x00, 0x06, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0xf0, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x7f, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x00, 0x00, 0x0f, 0x00, 0x00,
  0x00, 0xe0, 0xff, 0x03, 0x80, 0x3f, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x0f,
  0xe0, 0x7f, 0x00, 0x00, 0x00, 0xe0, 0xff, 0x1f, 0xf8, 0xff, 0x01, 0x00,
  0x00, 0xf0, 0xff, 0x7f, 0xfe, 0xff, 0x07, 0x00, 0x00, 0xe0, 0xff, 0xff,
  0xff, 0xff, 0x0f, 0x00, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0x03, 0x00,
  0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0xfc, 0xff,
  0xff, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0x1f, 0x00, 0x00,
  0x00, 0x00, 0xe0, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff,
  0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0,
  0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x07, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Heartrate logo: Width x Height = 10,10
static const unsigned char heart[] = {
  0xce, 0x01, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xff, 0x03, 0xfe, 0x01,
  0xfc, 0x00, 0x78, 0x00, 0x30, 0x00, 0x20, 0x00
};

// Step logo: Width x Height = 10,10
static const unsigned char mountain[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0xfc, 0x00,
  0xfe, 0x01, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Bluetooth logo: Width x Height = 10,10
static const unsigned char BT[] = {
  0x20, 0x00, 0x70, 0x00, 0xec, 0x00, 0xf8, 0x00, 0x70, 0x00, 0x70, 0x00,
  0xf8, 0x00, 0xec, 0x00, 0x70, 0x00, 0x20, 0x00
};

// Full battery logo: Width x Height = 10,10
static const uint8_t battHigh[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x01, 0xff, 0x03, 0xff, 0x03,
  0xff, 0x01, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Low battery logo: Width x Height = 10,10
static const uint8_t battLow[] = {
  0x00, 0x00, 0x00, 0x00, 0xff, 0x01, 0x03, 0x01, 0x02, 0x03, 0x03, 0x03,
  0x02, 0x01, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00
};

// Define display I2C bus
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// MPU 6050 status variables
MPU6050 mpu; // class default I2C address is 0x68
uint8_t mpuIntStatus; // holds interrupt status byte from MPU
uint16_t packetSize; // expected DMP packet size (42 bytes)
uint16_t fifoCount; // number of bytes in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// Callback function which determines if MPU interrupt has toggled
int mpu6050_int = 2; // interrupt pin I/O 2
volatile bool mpuInterrupt = false; // flag which will toggle when interrupt is called/handled
void dmpDataReady() {
  mpuInterrupt = true;
}

// Bluetooth and Clock Variables
BLEMate2 BTModu(&Serial1); // ble serial object
char dateBT[15]; // date info MM/DD/YYYY
char timeBT[15]; // time info HH:MM:SS MM
char callBT[20]; // caller number info
char textBT[20]; // text number info
bool isBTConnected; // used to determine if Bluetooth device is connected
bool isRTCInit; // determines if RTC has been initialized
unsigned long btLastConnected; // time which Bluetooth was last connected
RTCZero rtc; // instance of RTCZero class
byte lastSecondDisplayUpdated; // last second which display was updated (only update display every second)

// Step Counting Variables
Quaternion q; // quaternion container [w, x, y, z]
const double stepDiffMinThreshold = 0.025; // minimum difference between step max and min for considered step
int stepCount = 0; // total number of steps taken
double stepMax = 0; // peak of quaternion data
double stepMin = 0; // trough of quaternion data
double currentQData[3]; // array storing recent quaternion values

// Heart Rate Variables
MAX30105 heartRateSensor; // instance of MAX30105 class
const byte arraySizeHR = 5; // size of array containing latest HR values
byte heartRates[arraySizeHR]; // array containing latest HR values
byte heartRateIndex = 0; // index latest value was inputted into array
long prevHeartBeat = 0; // time at which the last heart beat occurred
float heartRate; // current heart rate
int heartRateAvg = 0; // average heart rate which will we be displayed
long timeLastHRBeat = 0;

// Battery Variables
const int batteryPin = 1; // analog 1: used to determine when battery is low/high
const float referenceVolts = 5.0; // the default reference on a 5-volt board

// SD Card Variables
File logFile; // instance of File class
const int sd_CS = 10; // chip select for SD card SPI

// Bootloader
void setup()
{
  Serial1.begin(9600);
  SerialUSB.begin(9600);
  Wire.begin();
  
  // Define displays I2C address and display boot screen
  u8g2.begin();
  u8g2.firstPage();
  do {
    u8g2.drawXBMP(32, 0, 64, 64, boot);
  } while ( u8g2.nextPage() );
  
  // setup MPU6050
  mpu.initialize(); // power up device and take out of sleep mode
  pinMode(mpu6050_int, INPUT);
  mpu.dmpInitialize(); // load DMP flash and configure the DMP
  mpu.setDMPEnabled(true); // turn on the DMP, now that it's ready

  // setup MPU6050 interrupt
  attachInterrupt(digitalPinToInterrupt(mpu6050_int), dmpDataReady, RISING);
  mpuIntStatus = mpu.getIntStatus();

  // get expected DMP packet size
  packetSize = mpu.dmpGetFIFOPacketSize();

  // initialize SD card
  SD.begin(sd_CS);
  
  // initialize Real Time Clock
  rtc.begin();

  // Turn on and setup heart rate sensor
  heartRateSensor.begin(Wire, I2C_SPEED_STANDARD); // 100 KHz
  heartRateSensor.setup(); // configure sensor with default settings
  heartRateSensor.setPulseAmplitudeRed(0x0A); // red LED to low to indicate sensor is running
  heartRateSensor.setPulseAmplitudeGreen(0); // turn off Green LED
  
  // Initialize BLE and set it into peripheral mode
  setupBLE();
}

// Function to update the display with latest information
void updateDisplay()
{
  // if there is an incoming phone call
  if (strlen(callBT) >= 10 && isBTConnected == true)
  {
    u8g2.setFont(u8g2_font_profont11_tf);
    // print time top left corner
    u8g2.drawStr(0, 10, timeBT);

    u8g2.setFont(u8g2_font_profont22_tf);
    // display call text and phone number
    u8g2.setCursor(40, 38);
    u8g2.print("Call");
    u8g2.drawStr(0, 58, callBT);
  }
  // if there is an incoming text
  else if (strlen(textBT) >= 10 && isBTConnected == true)
  {
    u8g2.setFont(u8g2_font_profont11_tf);
    // print time top left corner
    u8g2.drawStr(0, 10, timeBT);

    u8g2.setFont(u8g2_font_profont22_tf);
    // display text text and phone number
    u8g2.setCursor(40, 38);
    u8g2.print("Text");
    u8g2.drawStr(0, 58, textBT);
  }
  // if no incoming call or text
  else
  {
    u8g2.setFont(u8g2_font_profont11_tf);
    // display date
    if(isBTConnected == true)
      u8g2.drawStr(0, 10, dateBT);
    else
    {
      u8g2.setCursor(0, 10);
      u8g2.print(String(rtc.getMonth()) + "/" + String(rtc.getDay()) + "/20" + String(rtc.getYear()));
    }

    // display heart rate
    u8g2.setCursor(14, 62);
    u8g2.print(String(heartRateAvg) + " bpm");
    u8g2.drawXBMP(0, 54, 10, 10, heart);

    // display steps
    u8g2.setCursor(78, 62);
    u8g2.print(String(stepCount*2) + " stp");
    u8g2.drawXBMP(64, 54, 10, 10, mountain);

    // display time
    u8g2.setFont(u8g2_font_profont22_tf);
    // if connected to Bluetooth
    if(isBTConnected == true)
    {
      // center time on display
      if (strlen(timeBT) == 7)
        u8g2.drawStr(20, 38, timeBT);
      else if (strlen(timeBT) == 8)
        u8g2.drawStr(13, 38, timeBT);
    }
    // if not connected to Bluetooth
    else
    {
      u8g2.setCursor(13, 38);
      // time does not come with leading 0's, add to display string if needed
      if(rtc.getMinutes() < 10 && rtc.getSeconds() < 10)
        u8g2.print(String(rtc.getHours()) + ":0" + String(rtc.getMinutes()) + ":0" + String(rtc.getSeconds()));
      else if(rtc.getMinutes() < 10)
        u8g2.print(String(rtc.getHours()) + ":0" + String(rtc.getMinutes()) + ":" + String(rtc.getSeconds()));
      else if(rtc.getSeconds() < 10)
        u8g2.print(String(rtc.getHours()) + ":" + String(rtc.getMinutes()) + ":0" + String(rtc.getSeconds()));
      else
        u8g2.print(String(rtc.getHours()) + ":" + String(rtc.getMinutes()) + ":" + String(rtc.getSeconds()));
    }   
  }
  
  // draw BT logo only if connected
  if(isBTConnected == true)
    u8g2.drawXBMP(118, 0, 10, 10, BT);

  // obtain and display battery status
  int battVoltRaw = analogRead(batteryPin);
  float battVolt = (battVoltRaw / 1023.0) * referenceVolts;
  // display full battery
  if (battVolt >= 3.6)
    u8g2.drawXBMP(105, 0, 10, 10, battHigh);
  // display low battery
  if (battVolt < 3.6)
    u8g2.drawXBMP(105, 0, 10, 10, battLow);
}

void setRTCTime()
{
  byte setTimeDate;
  
  // set time (HH:MM PM)
  // determine how many bytes to read as format can be HH:MM or H:MM
  if(strlen(timeBT) == 7)
    setTimeDate = stringToByte(timeBT, 1);
  else
    setTimeDate = stringToByte(timeBT, 2);
  // determine if PM and add 12 hours if so
  if(memchr(timeBT, 'P', strlen(timeBT)) != NULL)
     rtc.setHours(setTimeDate + 12);
  else
    rtc.setHours(setTimeDate);
  // determine position of pointer (start of MM)as format can be HH:MM or H:MM
  if(strlen(timeBT) == 7)
    setTimeDate = stringToByte(timeBT+2, 2);
  else
    setTimeDate = stringToByte(timeBT+3, 2);
  rtc.setMinutes(setTimeDate);
  rtc.setSeconds(30);
  
  // set date (MM/DD/YYYY)
  setTimeDate = stringToByte(dateBT, 2);
  rtc.setMonth(setTimeDate);
  setTimeDate = stringToByte(dateBT+3, 2);
  rtc.setDay(setTimeDate);
  setTimeDate = stringToByte(dateBT+8, 2); // only get the last two digits of the year
  rtc.setYear(setTimeDate);
}

void countSteps()
{
  fifoCount = mpu.getFIFOCount();
  // if buffer is not empty or incomplete, reset
  if ((!fifoCount) || (fifoCount % packetSize)) 
  { 
    // we have failed the reset and will wait untill next time
    mpu.resetFIFO(); // clear the buffer
  } 
  else 
  {
    while (fifoCount  >= packetSize) 
    { 
      // get packets until we have the latest
      mpu.getFIFOBytes(fifoBuffer, packetSize); // lets do the magic and get the data
      fifoCount -= packetSize; // track FIFO count tp determine if more data can be read
    }

    // get quaternion values
    mpu.dmpGetQuaternion(&q, fifoBuffer);
  }

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

// Function used to calculate users heart rate
void calculateHR()
{
  // obtain infrared value
  long irValue = heartRateSensor.getIR();

  // if heart beat was detected and valid IR value
  if (checkForBeat(irValue) == true && irValue > 50000)
  {
    // calculate time difference between 2 beats
    long heartBeatTimeDiff = millis() - prevHeartBeat;
    prevHeartBeat = millis();

    // use the difference to calculate the heart rate
    heartRate = 60 / (heartBeatTimeDiff / 1000.0); // 60 s in 1 min, 1000 ms in 1 s

    // only use valid heart rates
    if (heartRate < 120 && heartRate > 40)
    {
      // store heart rate
      heartRates[heartRateIndex++] = (byte)heartRate;
      heartRateIndex %= arraySizeHR; // use modulus op. to determine current index

      // calcute average heart rate
      heartRateAvg = 0; // reset
      for (int i = 0; i < arraySizeHR; i++)
        heartRateAvg += heartRates[i]; // add up all heart rates
      heartRateAvg /= arraySizeHR; // determine average by dividing

      timeLastHRBeat = millis();
    }
  }
}

void loop()
{
  static String inputBuffer;

  // Read BLE input from UART
  while(Serial1.available() > 0)
  {
    inputBuffer.concat((char)Serial1.read());
  }

  // Check for leading RCV= and ending \n\r
  if(inputBuffer.endsWith("\n\r"))
  {
    if(inputBuffer.startsWith("RCV="))
    {
      // check to see what type of message
      if(inputBuffer.startsWith("RCV=D:"))
      {
        inputBuffer.remove(0,6);
        inputBuffer.toCharArray(dateBT, inputBuffer.length());
      }
      else if(inputBuffer.startsWith("RCV=T:"))
      {
        inputBuffer.remove(0,6);
        inputBuffer.remove(inputBuffer.length()-9, 3); // remove seconds
        inputBuffer.toCharArray(timeBT, inputBuffer.length());
      }
      else if(inputBuffer.startsWith("RCV=C:"))
      {
        inputBuffer.remove(0,6);
        inputBuffer.toCharArray(callBT, inputBuffer.length());
        str_replace(callBT, " ", "");
        str_replace(callBT, "(", "");
        str_replace(callBT, ")", "");
        str_replace(callBT, "-", "");
      }
      else if(inputBuffer.startsWith("RCV=M:"))
      {
        inputBuffer.remove(0,6);
        inputBuffer.toCharArray(textBT, inputBuffer.length());
        str_replace(textBT, " ", "");
        str_replace(textBT, "(", "");
        str_replace(textBT, ")", "");
        str_replace(textBT, "-", "");
      }
      
      // update display
      u8g2.firstPage();
      do {
        updateDisplay();
      } while ( u8g2.nextPage() );
      
      // clear buffer
      inputBuffer = "";

      // open log file
      logFile = SD.open("Esperto.txt", FILE_WRITE);
      // write date, time, HR, and step data to SD if its open
      if (logFile) 
      {
        logFile.write(dateBT, strlen(dateBT));
        logFile.print(" ");
        logFile.write(timeBT, strlen(timeBT));
        logFile.println(" | Step Count = " + String(stepCount*2) + " | Heart Rate = " + String(heartRate));
        logFile.close();
      }
    
      // update the time at which Bluetooth was last connected
      btLastConnected = millis();
      isBTConnected = true;
    }
    else
    {
      inputBuffer = "";
    }
  }
  if((isBTConnected == true || isRTCInit == true) && ((millis() - btLastConnected) > 10000) && lastSecondDisplayUpdated != rtc.getSeconds())
  {
    // set time based off last Bluetooth connection
    if(isBTConnected == true)
    {
      setRTCTime();
    }
    
    // update display
    u8g2.firstPage();
    do {
      updateDisplay();
    } while ( u8g2.nextPage() );

     // open log file
    logFile = SD.open("Esperto.txt", FILE_WRITE);
    // write date, time, HR, and step data to SD if its open
    if (logFile) 
    {
      logFile.print(String(rtc.getMonth()) + "/" + String(rtc.getDay()) + "/20" + String(rtc.getYear()));
      logFile.print(" " + String(rtc.getHours()) + ":" + String(rtc.getMinutes()) + ":" + String(rtc.getSeconds()));
      logFile.println(" | Step Count = " + String(stepCount*2) + " | Heart Rate = " + String(heartRate));
      logFile.close();
    }
    
    // update statuses
    isRTCInit = true;
    isBTConnected = false;
    lastSecondDisplayUpdated = rtc.getSeconds();
  }

  // wait for MPU interrupt to process steps
  if (mpuInterrupt)
  {
    mpuInterrupt = false; // reset interrupt flag
    countSteps();
  }
  
  // calculate heart rate
  if(millis()-timeLastHRBeat > 500)
  {
      calculateHR(); 
  }
}

// HELPER FUNCTIONS
// Initializes BLE and puts it into peripheral mode
void setupBLE()
{
  BTModu.reset(); // reset Module
  BTModu.restore(); // reset module to factory default
  BTModu.writeConfig(); // store current settings in non-volatile memory
  BTModu.reset(); // reset module so effects take change

  // Put the device in peripheral mode
  BTModu.stdSetParam("CCON", "ON"); // enable advertising
  BTModu.stdSetParam("ADVP", "FAST"); // control advertising rate
  BTModu.stdSetParam("ADVT", "0"); // set time before device times out (set to infinity)
  BTModu.stdSetParam("ADDR", "000000000000"); // control devices allowed to connect to us (set to all)
  BTModu.writeConfig(); // store settings
  BTModu.reset(); // reset module
}

// Function which replaces char(s) with new char(s) in a char array
void str_replace(char *src, char *oldchars, char *newchars)
{
  char *p = strstr(src, oldchars); // returns a pointer to the first occurrence of oldchars in src
  char buf[20];
  do {
    memset(buf, 0, strlen(buf));
    // until pointer points to null or zero
    if (p) {
      // if replacing char from beginining
      if (src == p) {
        strcpy(buf, newchars);
        strcat(buf, p + strlen(oldchars));
      } else {
        strncpy(buf, src, strlen(src) - strlen(p));
        strcat(buf, newchars);
        strcat(buf, p + strlen(oldchars));
      }
      memset(src, 0, strlen(src));
      strcpy(src, buf);
    }
  } while (p && (p = strstr(src, oldchars)));
}

// Function which convert a number in a char array to a byte data type
byte stringToByte(char *src, int numBytes)
{
  // intermediate buffers used during conversions
  char charBuffer[4];

  memcpy(charBuffer, src, numBytes);

  // cast from char[] to int to byte
  return (byte)atoi(charBuffer);
}

// Function which determines amount of free SRAM
int freeRam () 
{
  char stack_dummy = 0;
  return &stack_dummy - sbrk(0);
}
