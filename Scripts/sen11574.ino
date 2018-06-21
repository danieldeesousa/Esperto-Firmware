// Heart Rate Variables
int ppgPin = 0; // analog 0
int ppgSignal; // signal which we obtain from sensor
const int ppgThreshold = 300; // threshold in mV which determines when heart beats
unsigned long time1 = 0; // previous heart beat time
unsigned long time2 = 0; // current heart beat time
int heartRate;
const int timeArraySize = 10;
int avTime[timeArraySize]; // array storing running HR total
int timeSum = 0; // sum used to find average of array

// Bootloader
void setup() 
{
  Serial.begin(9600);
}

// function used to calculate users heart rate
void calculateHR()
{
  ppgSignal = analogRead(ppgPin);
  // we only process incoming signal if it reaches threshold
  if(ppgSignal >= ppgThreshold)
  {
    // set the times at which the beats occur and calculate the time between 2 beats
    time1 = time2;
    time2 = millis();
    int timeDiff = int(time2-time1); // time between consecutive beats
    // only look at consecutive heart beats between 60 and 120 bpm to remove any noise
    if(timeDiff>=500 && timeDiff<=1000)
    {
      // update running total array and calculate its sum
      for(int i=0; i<(timeArraySize-1); i++)
      {
        avTime[i] = avTime[i+1];
        timeSum += avTime[i];
      }
      avTime[timeArraySize-1] = timeDiff;
      timeSum += timeDiff;
      // only calculate heart rate if there has been at least 10 heart beats
      if(avTime[0]!=0)
      {
        heartRate = 60000*10/(timeSum);
      }
      // reset sum
      timeSum = 0;
      //Serial.println(heartRate);
    }
  }
}

void loop() 
{
  ppgSignal = analogRead(ppgPin);
  Serial.println(ppgSignal);
  // only calculate HR if 0.2s has passed since the last beat to avoid looking at the same peak
  if((millis()-time2)>200)
  {
    //calculateHR();
  }
}
