/*
The MIT License (MIT)

Copyright (c) 2015 Technical Rockstars
Copyright (C) 2015 Embedded and Real-Time Systems Laboratory
			  Graduate School of Information Science, Nagoya Univ., JAPAN
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "Nefry_Milkcocoa.h"
#include "stdio.h"


DataElement::DataElement() {
	params = aJson.createObject();
	paJsonObj = aJson.createObject();
	aJson.addItemToObject(paJsonObj, "params", params);
}

DataElement::DataElement(char *json_string) {
	paJsonObj = aJson.parse(json_string);
	params = aJson.getObjectItem(paJsonObj, "params");
}

DataElement::~DataElement() {
	aJson.deleteItem(paJsonObj);
	paJsonObj = NULL;
	params = NULL;
}

void DataElement::setValue(const char *key, const char *v) {
	aJson.addStringToObject(params, key, v);
}

void DataElement::setValue(const char *key, int v) {
	aJson.addNumberToObject(params, key, v);
}

void DataElement::setValue(const char *key, double v) {
	aJson.addNumberToObject(params, key, v);
}

char *DataElement::getString(const char *key) {
	aJsonObject* obj = aJson.getObjectItem(params, key);
	if (obj == NULL) {
		Serial.println("obj is NULL");
		return (char*)"";
	}
	return obj->valuestring;
}

int DataElement::getInt(const char *key) {
	aJsonObject* obj = aJson.getObjectItem(params, key);
	if (obj == NULL) {
		Serial.println("obj is NULL");
		return 0;
	}
	return obj->valueint;
}

float DataElement::getFloat(const char *key) {
	aJsonObject* obj = aJson.getObjectItem(params, key);
	if (obj == NULL) {
		Serial.println("obj is NULL");
		return 0;
	}
	return obj->valuefloat;
}


char *DataElement::toCharArray() {
	return aJson.print(paJsonObj);
}



Nefry_Milkcocoa::Nefry_Milkcocoa() {}

Nefry_Milkcocoa::Nefry_Milkcocoa(Client *client, uint16_t port, const char *_app_id, const char *_session) {
	sprintf(session, "%s", _session);
	app_id = _app_id;
	randomSeed(analogRead(A0));
	sprintf(_app_id_, "%s.mlkcca.com", _app_id);
	sprintf(_client_id, "%s%d%s", __TIME__, random(99999), _app_id);
	delay(50);
	mqtt = new Adafruit_MQTT_Client(client, _app_id_, port, _client_id, session, app_id);

	for (uint8_t i = 0; i < MILKCOCOA_SUBSCRIBERS; i++) {
		milkcocoaSubscribers[i] = 0;
	}
}

WiFiClient _client;
Nefry_Milkcocoa* Nefry_Milkcocoa::begin(const char * appid, const char *key, const char *secret)
{
	if (strlen(appid) != 0 && strlen(key) != 0 && strlen(secret) != 0) {
		char _session[60];
		sprintf(_session, "k%s:%s", key, secret);
		delay(20);
		
		return new Nefry_Milkcocoa(&_client, 1883, appid, _session);
	} else {
		Nefry.println("Milkcocoa Send Data Lack");
		while (true){
			Nefry.setLed(255, 0, 0);
			Nefry.ndelay(10);
		}
	}

}

bool Nefry_Milkcocoa::connect(uint16_t timeout) {
	int ret;
	int cnt = 0;

	if (mqtt->connected()) {
		return true;
	}

	Serial.print("Connecting to MQTT... ");

	while ((ret = mqtt->connect()) != 0) { // connect will return 0 for connected
		Serial.println(mqtt->connectErrorString(ret));
		Serial.println(ret);
		Serial.println("Retrying MQTT connection in 5 seconds...");
		mqtt->disconnect();
		Nefry.ndelay(5000);  // wait 5 seconds
		if (timeout > 0) {
			cnt++;
			if (cnt * 5000 >= timeout) return false;
		}
	}
	Serial.println("MQTT Connected!");
	return true;
}

bool Nefry_Milkcocoa::push(const char *path, DataElement *pdataelement) {
	char topic[100];
	bool ret;
	char *send_array;
	sprintf(topic, "%s/%s/push", app_id, path);
	Adafruit_MQTT_Publish pushPublisher = Adafruit_MQTT_Publish(mqtt, topic);
	send_array = pdataelement->toCharArray();
	ret = pushPublisher.publish(send_array);
	free(send_array);
	return ret;
}

bool Nefry_Milkcocoa::send(const char *path, DataElement *pdataelement) {
	char topic[100];
	bool ret;
	char *send_array;
	sprintf(topic, "%s/%s/send", app_id, path);
	Adafruit_MQTT_Publish pushPublisher = Adafruit_MQTT_Publish(mqtt, topic);
	send_array = pdataelement->toCharArray();
	ret = pushPublisher.publish(send_array);
	free(send_array);
	return ret;
}
void Nefry_Milkcocoa::print() {
	Nefry.setConfHtml("AppID", 0);
	Nefry.setConfHtml("APIKey", 1);
	Nefry.setConfHtml("APISecret", 2);
	Nefry.setConfHtml("DataStore", 3);
}
bool Nefry_Milkcocoa::loop(uint16_t timeout) {
	if (!connect(timeout)) {
		return false;
	}
	Adafruit_MQTT_Subscribe *subscription;
	while ((subscription = mqtt->readSubscription(1000))) {
		for (uint8_t i = 0; i < MILKCOCOA_SUBSCRIBERS; i++) {
			if (milkcocoaSubscribers[i] == 0) continue;
			if (subscription == (milkcocoaSubscribers[i]->mqtt_sub)) {
				DataElement de = DataElement((char *)milkcocoaSubscribers[i]->mqtt_sub->lastread);
				milkcocoaSubscribers[i]->cb(&de);
			}
		}
	}
	return true;
}

bool Nefry_Milkcocoa::on(const char *path, const char *event, GeneralFunction cb) {
	MilkcocoaSubscriber *sub = new MilkcocoaSubscriber(cb);
	sprintf(sub->topic, "%s/%s/%s", app_id, path, event);

	uint8_t i;
	Adafruit_MQTT_Subscribe *mqtt_sub = new Adafruit_MQTT_Subscribe(mqtt, sub->topic);
	sub->set_mqtt_sub(mqtt_sub);
	if (!mqtt->subscribe(mqtt_sub)) {
		return false;
	}
	for (i = 0; i < MILKCOCOA_SUBSCRIBERS; i++) {
		if (milkcocoaSubscribers[i] == sub) {
			return false;
		}
	}
	for (i = 0; i < MILKCOCOA_SUBSCRIBERS; i++) {
		if (milkcocoaSubscribers[i] == 0) {
			milkcocoaSubscribers[i] = sub;
			return true;
		}
	}
	return false;
}

MilkcocoaSubscriber::MilkcocoaSubscriber(GeneralFunction _cb) {
	cb = _cb;
}

void MilkcocoaSubscriber::set_mqtt_sub(Adafruit_MQTT_Subscribe *_mqtt_sub) {
	mqtt_sub = _mqtt_sub;
}

