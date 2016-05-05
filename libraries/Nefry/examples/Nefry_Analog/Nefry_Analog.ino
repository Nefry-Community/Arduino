#include <Nefry.h>
int sensorValue;
void setup() {
}
void loop() {
  // read the analog in value:
  sensorValue = analogRead(A0);
  Nefry.print("sensor = " );
  Nefry.println(sensorValue);
  delay(1000);
}
