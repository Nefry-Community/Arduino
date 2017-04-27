#include <Nefry.h>
#include <NefryIFTTT.h>
String Event, SecretKey;

void setup() {
  Nefry.setConfHtmlStr("SecretKey",0); //Nefry DataStoreのタイトルを指定
  Nefry.setConfHtmlStr("Event",1);     //Nefry DataStoreのタイトルを指定
  SecretKey = Nefry.getConfStr(0);     //Nefry DataStoreからデータを取得
  Event = Nefry.getConfStr(1);         //Nefry DataStoreからデータを取得
}
 
void loop() {
  if (Nefry.readSW()) {               //SWを押した時
    if (!IFTTT.send(Event, SecretKey)) {//IFTTTにデータを送信
      Nefry.setLed(255, 0, 0);        //Errの時、赤色点灯
    }
    Nefry.ndelay(1000);               //送信後1秒間待つ
  }
}