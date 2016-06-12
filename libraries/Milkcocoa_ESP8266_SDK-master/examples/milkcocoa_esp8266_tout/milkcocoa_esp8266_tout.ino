#include <ESP8266WiFi.h>
#include <Milkcocoa.h>

/************************* Your Milkcocoa Setup *********************************/

#define MILKCOCOA_APP_ID      "...YOUR_MILKCOCOA_APP_ID..."
#define MILKCOCOA_DATASTORE   "nefry"

/************ Global State (you don't need to change this!) ******************/

WiFiClient client;

const char MQTT_SERVER[] PROGMEM    = MILKCOCOA_APP_ID ".mlkcca.com";
const char MQTT_CLIENTID[] PROGMEM  = __TIME__ MILKCOCOA_APP_ID;

Milkcocoa milkcocoa = Milkcocoa(&client, MQTT_SERVER,1883, MILKCOCOA_APP_ID, MQTT_CLIENTID);
void onpush(DataElement *elem);
void setup() {

  Serial.println( milkcocoa.on(MILKCOCOA_DATASTORE, "push", onpush) );
};

void loop() {
  milkcocoa.loop();
  DataElement elem = DataElement();

  elem.setValue("v", analogRead(A0));
  milkcocoa.push(MILKCOCOA_DATASTORE, &elem);
  delay(7000);
};

void onpush(DataElement *elem) {
  Serial.println("onpush");
  Serial.println(elem->getInt("v"));
};
