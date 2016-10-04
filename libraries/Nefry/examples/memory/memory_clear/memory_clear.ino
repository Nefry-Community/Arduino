#include <Nefry.h>
void setup() {
  Nefry.setConfWifi("Nefry","");//SSID:PASS
  Nefry.setConfModule("","","");//ModuleID:ModuleClass:ModuleWiFiPass
  Nefry.setConfUser("","");//user:pass
  for (int i = 0; i < 8; i++) {
    Nefry.setConfValue(0, i);
    Nefry.setConfStr("", i);
  }
}

void loop() {

}
