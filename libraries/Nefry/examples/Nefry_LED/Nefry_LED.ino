#include <Nefry.h>
//フルカラーLED　ランダムにカラーが変わります。
void setup() {
  Nefry.println("フルカラーLED!");
  randomSeed(analogRead(A0));
}
int red,green,blue;
void loop() {
  red=random(255);//random関数は0-255の数値をランダムに返します。
  green=random(255);
  blue=random(255);
  Nefry.setLed(red,green,blue);//LEDがランダムに点灯します。
  String color="Red:";color+=red;
  color+=" Green:";color+=green;
  color+=" Blue:";color+=blue;
  Nefry.println(color);//Nefry consoleで色を表示
  Nefry.ndelay(1000);//1秒待つ
}
