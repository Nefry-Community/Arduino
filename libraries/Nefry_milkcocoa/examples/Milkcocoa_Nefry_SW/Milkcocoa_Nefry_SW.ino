#include <Nefry.h>
#include <Nefry_Milkcocoa.h>

Nefry_Milkcocoa *milkcocoa;

char *datastore;
void onpush(DataElement *elem);

void setup() {
  milkcocoa->print();//Setup ModuleにAPPIDの表示をする
  milkcocoa = Nefry_Milkcocoa::begin();
  datastore = Nefry.getConfStr(3);
  Nefry.println( milkcocoa->on( datastore, "push", onpush) ? "Milkcocoa onPush OK" : "Milkcocoa onPush NG" );
}
bool flg, old;
void loop() {
  if ( (flg = Nefry.push_SW()) != old) {//ボタンの状態が前回と異なれば
    Nefry.println( milkcocoa->loop(10000)? "Milkcocoa Connect OK" : "Milkcocoa Connect NG" );
    DataElement elem = DataElement();
    elem.setValue("sw", flg );
    old = flg;
    milkcocoa->push(datastore, &elem);
  }
}

void onpush(DataElement *elem) {//MilkcocoaのOn関数と同一
  Nefry.print("onpush:");
  Nefry.println(elem->getInt("sw"));
}

