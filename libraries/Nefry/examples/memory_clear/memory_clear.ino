#include <Nefry.h>
void setup() {
  Nefry.setWifiConf("Nefry","Nefry-wifi");
  Nefry.setModuleConf("","","");
  Nefry.setUserConf("","");
  for (int i = 0; i < 8; i++) {
    Nefry.memory_write_mode(0, i);
    Nefry.memory_write("", i + 1);
  }
}

void loop() {

}
