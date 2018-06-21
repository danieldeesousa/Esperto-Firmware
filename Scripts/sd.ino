#include <SD.h>
 
File logFile;
const int sd_CS = 10;


int heartRate = 82;
int stepCount = 1401;
char dateBT[15] = "09/11/2001";
char timeBT[15] = "20:30:02";

void setup() 
{
  Serial.begin(9600);
  SD.begin(sd_CS);
  logFile = SD.open("Esperto.txt", FILE_WRITE);

  // write to SD if its open
  if (logFile) 
  {
    logFile.write(dateBT, strlen(dateBT));
    logFile.print(" ");
    logFile.write(timeBT, strlen(timeBT));
    logFile.println(": Step Count = " + String(stepCount) + " Heart Rate = " + String(heartRate));
    logFile.close();
  }
}

void loop() 
{

}
