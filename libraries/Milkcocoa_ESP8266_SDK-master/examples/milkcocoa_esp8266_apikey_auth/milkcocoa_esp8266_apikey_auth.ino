#include <ESP8266WiFi.h>
#include <Milkcocoa.h>

/************************* Your Milkcocoa Setup *********************************/

#define MILKCOCOA_APP_ID      "...YOUR_MILKCOCOA_APP_ID..."
#define MILKCOCOA_DATASTORE   "esp8266"
#define MILKCOCOA_API_KEY     "...YOUR_MILKCOCOA_API_KEY..."
#define MILKCOCOA_API_SECRET  "...YOUR_MILKCOCOA_API_SECRET..."

/************ Global State (you don't need to change this!) ******************/

WiFiClient client;

const char MQTT_SERVER[] PROGMEM    = MILKCOCOA_APP_ID ".mlkcca.com";
const char MQTT_CLIENTID[] PROGMEM  = __TIME__ MILKCOCOA_APP_ID;

Milkcocoa *milkcocoa = Milkcocoa::createWithApiKey(&client, MQTT_SERVER, 1883, MILKCOCOA_APP_ID, MQTT_CLIENTID, MILKCOCOA_API_KEY, MILKCOCOA_API_SECRET);
void onpush(DataElement *elem);
void setup() {

  Serial.println( milkcocoa->on(MILKCOCOA_DATASTORE, "push", onpush) );
};

void loop() {
  milkcocoa->loop();

  DataElement elem = DataElement();
  elem.setValue("v", 1);

  milkcocoa->push(MILKCOCOA_DATASTORE, &elem);
  delay(7000);
};

void onpush(DataElement *elem) {
  Serial.println("onpush");
  Serial.println(elem->getInt("v"));
};

