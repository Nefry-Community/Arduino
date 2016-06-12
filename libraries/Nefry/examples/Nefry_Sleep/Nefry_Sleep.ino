#include <Nefry.h>
//Sleepモードの例　30秒したらスリープから復帰します。
void setup() {
}
void loop() {
  Nefry.println("Sleep!");
  Nefry.setLed(0, 0, 255);//青色に点灯します。
  delay(5000);
  Nefry.setLed(0, 0, 0, 0); //消灯します。
  Nefry.sleep(30);//秒数で指定します。
}

