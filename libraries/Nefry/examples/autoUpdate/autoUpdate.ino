#include<Nefry.h>
void setup() {
  Nefry.setProgramName("autoUpdate");//プログラム名表示
  int i=Nefry.autoUpdate("nfry.hol.es","/nefry");//ドメイン,保存場所
  if(i==true){
    Nefry.println("確認しましたが更新はありません。");
  }else{
    Nefry.print("更新に問題が発生しました。ErrCode : ");
    Nefry.println(i);
    //ErrCode
  }
}

void loop() {
  // put your main code here, to run repeatedly:
Nefry.setLed(random(255),random(255),random(255));
}
