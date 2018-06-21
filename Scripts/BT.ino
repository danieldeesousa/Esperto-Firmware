
String message; //string that stores the incoming message

void setup()
{
  Serial.begin(9600); //set baud rate
}

void loop()
{
  if(Serial.available() > 0){
    
    //READ THE STRING TO THE FIRST "|" DIVIDER AND STORE AS A VARIABLE
    String myDate = Serial.readStringUntil('|');
    Serial.read();
    Serial.println(myDate);
    //STORE THE NEXT SECTION OF STRING AS A VARIABLE
    String myTime = Serial.readStringUntil('|');
    Serial.read();
    Serial.println(myTime);
    //STORE THE THIRD SECTION OF STRING AS A VARIABLE
    String myPhone = Serial.readStringUntil('|');
    Serial.read();
    Serial.println(myPhone);
    //STORE THE FINAL SECTION AS A VARIABLE
    String myText  = Serial.readStringUntil('\n');
    Serial.println(myText);
  }
}
