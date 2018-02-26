void setup()
{
  Serial.begin(9600);
  delay(5000);
  Serial.print("AT+NAMEEspertoWatch");
  delay(2000);
}

void loop()
{
}
