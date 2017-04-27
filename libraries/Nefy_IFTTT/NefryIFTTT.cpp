/**
Nefry IFTTT lib

Copyright (c) 2015 wami

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/

#include "NefryIFTTT.h"

bool Nefry_IFTTT::send(String event, String Secretkey, String data1, String data2, String data3)
{
	String _data = "{\"value1\":\"";
	_data += data1;
	_data += "\",\"value2\":\"";
	_data += data2;
	_data += "\",\"value3\":\"";
	_data += data3;
	_data += "\"}";
	WiFiClient client;
	if (client.connect("maker.ifttt.com", 80)) {
		//Nefry.println("connection");
		client.println("POST /trigger/" + event + "/with/key/" + Secretkey + " HTTP/1.1");
		client.println("Host: maker.ifttt.com");
		client.println("User-Agent: ESP8266/1.0");
		client.println("Connection: close");
		client.println("Content-Type: application/json");
		client.print("Content-Length: ");
		client.println(_data.length());
		client.println();
		client.println(_data);
		delay(10);
		return true;
	}
	return false;
}
Nefry_IFTTT IFTTT;
