#include <Nefry.h>
void setup() {
  pinMode(D5,OUTPUT);//出力モード切り替え
  pinMode(D4,INPUT_PULLUP);//入力モード切り替え
}
void loop() {
  //SWを押した時にD5にLEDを差すと点灯する
  if(Nefry.push_SW()){
    //押した時
    digitalWrite(D5,HIGH);
    Nefry.println("Push SW");
  }else{
    //離した時
    digitalWrite(D5,LOW);
  }
  
  //D4がLOWになったときにNefryのLEDが青色に光る
  if(digitalRead(D4)==LOW){
    //赤色になる（r,g,b）の順で色を指定できる
    Nefry.Nefry_LED(0,0,255);
    Nefry.println("D4 LOW");
  }else{
    //緑色になる
    Nefry.Nefry_LED(0,255,255);
  } 
  delay(100);
}

