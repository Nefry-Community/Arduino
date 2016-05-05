#include <Nefry.h>
void setup() {
}
void loop() {
  //SWを押した時にLEDが赤色に点灯する
  if(Nefry.push_SW()){
    //押した時
    //赤色になる（r,g,b）の順で色を指定できる
    Nefry.Nefry_LED(0,0,255);
    Nefry.println("Push SW");
  }else{
    //離した時
    Nefry.Nefry_LED(0,255,255);
  }
  delay(100);
}

