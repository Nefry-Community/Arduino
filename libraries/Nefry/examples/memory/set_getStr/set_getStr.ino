#include<Nefry.h>
void setup() {
  Nefry.setConfStr("Hello World",0);
  Nefry.setConfStr("welcome to Nefry", 1);
  Nefry.println(Nefry.getConfStr(0));
  Nefry.println(Nefry.getConfStr(1));
}

void loop() {
}
