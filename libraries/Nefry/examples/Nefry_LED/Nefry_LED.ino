#include <Nefry.h>
//フルカラーLED　ランダムにカラーが変わります。
void setup() {
  Nefry.println("フルカラーLED!");
  randomSeed(analogRead(A0));
}
void loop() {
  Nefry.Nefry_LED(random(255),random(255),random(255));//ランダムに点灯します。
  Nefry.ndelay(1000);
}

