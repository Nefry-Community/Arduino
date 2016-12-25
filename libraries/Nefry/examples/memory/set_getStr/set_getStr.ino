#include<Nefry.h>
void setup() {
  Nefry.setConfHtmlStr("str0", 0);
  Nefry.setConfHtmlStr("str1", 1);
  Nefry.setConfStr("Hello World", 0);
  Nefry.setConfStr("welcome to Nefry", 1);
  Nefry.println(Nefry.getConfStr(0));
  Nefry.println(Nefry.getConfStr(1));
}

void loop() {
}
