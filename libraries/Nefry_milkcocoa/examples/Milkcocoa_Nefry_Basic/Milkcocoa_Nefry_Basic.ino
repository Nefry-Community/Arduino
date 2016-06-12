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

void loop() {
  Nefry.println( milkcocoa->loop(10000) ? "Milkcocoa Connect OK" : "Milkcocoa Connect NG" );
  DataElement elem = DataElement();
  elem.setValue("char", "hello world");
  elem.setValue("int", 100);
  elem.setValue("float", 1.2);
  milkcocoa->push(datastore, &elem);
  Nefry.ndelay(5000);
}

void onpush(DataElement *elem) {//MilkcocoaのOn関数と同一
  Nefry.print("onpush:");
  Nefry.println(elem->getInt("int"));
  Nefry.println(elem->getFloat("float"));
  Nefry.println(elem->getString("char"));
}

