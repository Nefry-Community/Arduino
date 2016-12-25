#include <Nefry.h>
void setup() {
  Nefry.setConfModule("", "", ""); //ModuleID:ModuleClass:ModuleWiFiPass
  Nefry.setConfUser("", ""); //user:pass
  for (int i = 0; i < 8; i++) {
    Nefry.setConfValue(0, i);
    Nefry.setConfStr("", i);
  }
  for (int i = 0; i < 5; i++) {
    Nefry.deleteWifi(i);
  }
  Nefry.setConfWifi("Nefry", ""); //SSID:PASS
}

void loop() {

}
