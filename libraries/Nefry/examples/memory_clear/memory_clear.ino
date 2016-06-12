#include <Nefry.h>
void setup() {
  Nefry.setConfWifi("Nefry","Nefry-wifi");
  Nefry.setConfModule("","","");
  Nefry.setConfUser("","");
  for (int i = 0; i < 8; i++) {
    Nefry.setConfValue(0, i);
    Nefry.setConfStr("", i + 1);
  }
}

void loop() {

}
