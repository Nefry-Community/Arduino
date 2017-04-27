#include <Nefry.h>
#include <NefryIFTTT.h>
String Event, SecretKey;
int counter =0;                       //送信データのカウンタ

void setup() {
  Nefry.setConfHtmlStr("SecretKey",0); //Nefry DataStoreのタイトルを指定
  Nefry.setConfHtmlStr("Event",1);     //Nefry DataStoreのタイトルを指定
  SecretKey = Nefry.getConfStr(0);     //Nefry DataStoreからデータを取得
  Event = Nefry.getConfStr(1);         //Nefry DataStoreからデータを取得
}
 
void loop() {
  if (Nefry.readSW()) {               //SWを押した時
    counter++;                        //送信回数加算
    bool sendData = IFTTT.send(Event, SecretKey,"Nefry",(String)(micros()/1000000)+"秒",(String)counter);//IFTTTにデータを送信
                                      //Value1:Nefry,Value2:Nefryが起動してからの秒数,Value3:送信カウンタ
    if (!sendData) {//IFTTTにデータを送信が成功したか失敗したかの判定
      Nefry.setLed(255, 0, 0);        //Errの時、赤色点灯
    }
    Nefry.ndelay(1000);               //送信後1秒間待つ
  }
}