#include <SparkFunBLEMate2.h>
#include <SAMD_AnalogCorrection.h>

BLEMate2 BTModu(&Serial1);
char dateBT[15]; // date info MM/DD/YYYY
char timeBT[15]; // time info HH:MM:SS MM
char callBT[20]; // caller number info
char textBT[20]; // text number info

void setup()
{
  SerialUSB.begin(9600);
  Serial1.begin(9600);

  setupBLE();
}

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

void loop()
{
  static String inputBuffer;
  while(Serial1.available() > 0)
  {
    inputBuffer.concat((char)Serial1.read());
  }
  // Only read strings which start with RCV and end with \n\r
  if (inputBuffer.endsWith("\n\r"))
  {
    if (inputBuffer.startsWith("RCV="))
    {
      inputBuffer.trim(); // Remove \n\r from end.
      inputBuffer.remove(0,4); // Remove RCV= from front.
      if(inputBuffer.startsWith("D"))
      {
        inputBuffer.remove(0,2);
        SerialUSB.println(inputBuffer);
      }
      else if(inputBuffer.startsWith("T"))
      {
        inputBuffer.remove(0,2);
        SerialUSB.println(inputBuffer);
      }
      else if(inputBuffer.startsWith("C"))
      {
        inputBuffer.remove(0,2);
        SerialUSB.println(inputBuffer);
      }
      else if(inputBuffer.startsWith("M"))
      {
        inputBuffer.remove(0,2);
        SerialUSB.println(inputBuffer);
      }
      inputBuffer = "";
    }
    else
    {
      inputBuffer = "";
    }
  }
}

/*void loop()
{
  char input[100];
  char ending[] = "\n\r";
  char lead[] = "RCV=";
  char type[] = "DTCM";
  
  // Read BLE input from UART
  while(Serial1.available() > 0)
  {
    char i = (char)Serial1.read();
    memset(input+strlen(input), i, 1);
    memset(input+strlen(input)+1, '\0', 1);
    memset(&i, 0, 1);
  }

  // Check for leading RCV= and ending \n\r
  if(strcmp(input+(strlen(input)-2), ending) == 0)
  {
    if(strncmp(input, lead, 4) == 0)
    {
      // Remove leading RCV= and ending \n\r
      for(int i = 0+0; i <= (strlen(input)-4); i++)
      {
        input[i] = input[i+4];
      }
      memset(input+(strlen(input)-2), '\0', 1);

      if(strncmp(input, type, 1) == 0)
        strcpy(dateBT, input+2);
      else if(strncmp(input, type+1, 1) == 0)
        strcpy(timeBT, input+2);
      else if(strncmp(input, type+2, 1) == 0)
        strcpy(callBT, input+2);
      else if(strncmp(input, type+3, 1) == 0)
        strcpy(textBT, input+2);

      isBTConnected = true;
    
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
        logFile.write(dateBT, strlen(dateBT));
        logFile.print(" ");
        logFile.write(timeBT, strlen(timeBT));
        logFile.println(" | Step Count = " + String(stepCount*2) + " | Heart Rate = " + String(heartRate));
        logFile.close();
      }
      
      // update the time at which Bluetooth was last connected
      btLastConnected = millis();
    
      // Reset input buffer
      memset(input, 0, strlen(input));
    }
    else
    {
      // Reset input buffer
      memset(input, 0, strlen(input));
    }
  }
}*/
