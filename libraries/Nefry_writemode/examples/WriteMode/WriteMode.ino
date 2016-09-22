#include <Nefry.h>
#include<NefryWriteMode.h>

void WriteModeSetup() {
  Nefry.println("Write Mode Setup");
}
void WriteModeloop() {
  Nefry.setLed(0, random(255), 0);
  Nefry.ndelay(500);
}
NefryWriteMode WriteMode(WriteModeSetup, WriteModeloop);
void setup() {
  Nefry.println("Setup");
}

void loop() {
  // put your main code here, to run repeatedly:
  Nefry.setLed(random(250), random(255), random(255));
  Nefry.ndelay(500);
  Nefry.println(random(250));
}


