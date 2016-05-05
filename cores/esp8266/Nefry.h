/*
Nefry.h - Nefry Library

Copyright (c) 2015 wami. All rights reserved.
This file is part of the esp8266 core and Nefry library for Arduino environment.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#ifndef Nefry_h
#define Nefry_h
#include <ESP.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266httpUpdate.h>	

#define WIFI_CONF_FORMAT {0, 0, 0, 1}
#define NAME_PREF "Nefry-"
#define WIFI_CONF_START 0

#define NEO_KHZ800 0x0000 
// RGB NeoPixel permutations; white and red offsets are always same
// Offset:         W          R          G          B
#define NEO_RGB  ((0 << 6) | (0 << 4) | (1 << 2) | (2))
#define NEO_RBG  ((0 << 6) | (0 << 4) | (2 << 2) | (1))
#define NEO_GRB  ((1 << 6) | (1 << 4) | (0 << 2) | (2))
#define NEO_GBR  ((2 << 6) | (2 << 4) | (0 << 2) | (1))
#define NEO_BRG  ((1 << 6) | (1 << 4) | (2 << 2) | (0))
#define NEO_BGR  ((2 << 6) | (2 << 4) | (1 << 2) | (0))

// RGBW NeoPixel permutations; all 4 offsets are distinct
// Offset:         W          R          G          B
#define NEO_WRGB ((0 << 6) | (1 << 4) | (2 << 2) | (3))
#define NEO_WRBG ((0 << 6) | (1 << 4) | (3 << 2) | (2))
#define NEO_WGRB ((0 << 6) | (2 << 4) | (1 << 2) | (3))
#define NEO_WGBR ((0 << 6) | (3 << 4) | (1 << 2) | (2))
#define NEO_WBRG ((0 << 6) | (2 << 4) | (3 << 2) | (1))
#define NEO_WBGR ((0 << 6) | (3 << 4) | (2 << 2) | (1))

#define NEO_RWGB ((1 << 6) | (0 << 4) | (2 << 2) | (3))
#define NEO_RWBG ((1 << 6) | (0 << 4) | (3 << 2) | (2))
#define NEO_RGWB ((2 << 6) | (0 << 4) | (1 << 2) | (3))
#define NEO_RGBW ((3 << 6) | (0 << 4) | (1 << 2) | (2))
#define NEO_RBWG ((2 << 6) | (0 << 4) | (3 << 2) | (1))
#define NEO_RBGW ((3 << 6) | (0 << 4) | (2 << 2) | (1))

#define NEO_GWRB ((1 << 6) | (2 << 4) | (0 << 2) | (3))
#define NEO_GWBR ((1 << 6) | (3 << 4) | (0 << 2) | (2))
#define NEO_GRWB ((2 << 6) | (1 << 4) | (0 << 2) | (3))
#define NEO_GRBW ((3 << 6) | (1 << 4) | (0 << 2) | (2))
#define NEO_GBWR ((2 << 6) | (3 << 4) | (0 << 2) | (1))
#define NEO_GBRW ((3 << 6) | (2 << 4) | (0 << 2) | (1))

#define NEO_BWRG ((1 << 6) | (2 << 4) | (3 << 2) | (0))
#define NEO_BWGR ((1 << 6) | (3 << 4) | (2 << 2) | (0))
#define NEO_BRWG ((2 << 6) | (1 << 4) | (3 << 2) | (0))
#define NEO_BRGW ((3 << 6) | (1 << 4) | (2 << 2) | (0))
#define NEO_BGWR ((2 << 6) | (3 << 4) | (1 << 2) | (0))
#define NEO_BGRW ((3 << 6) | (2 << 4) | (1 << 2) | (0))

class Nefry_lib
{
public:
	bool pushSW_flg;
	String network_html, network_list, input_console;
	char module_input[20][15];
	void
		reset(),
		sleep(const int sec),
		setWifiConf(const char SSID[32], const char pass[64]),
		setModuleConf(const char module_id_[32], const char module_class_[32], const char module_wifi_pass_[64]),
		setUserConf(const char user[32], const char pass[32]),
		setHtmlConf(const char set[15],const int num),
		Nefry_LED_begin(const int num,const int pin, uint8_t t),
		Nefry_LED(const char r, char g,const char b,const char w = 122,const char pin = 0,const int num = 0),
		Nefry_LED_blink(const char r,const char g,const char b,const int wait,const int loop,const char pin = 0),
		println(float text),
		println(double text),
		println(char text),
		println(int text),
		println(long text),
		println(unsigned char text),
		println(unsigned int text),
		println(unsigned long text),
		print(float text),
		print(double text),
		print(char text),
		print(int text),
		print(long text),
		print(unsigned char text),
		print(unsigned int text),
		print(unsigned long text),
		print(String text),
		println(String text),
		nefry_init(),
		nefry_loop(),
		ndelay(unsigned long ms);
	int available(),
		Nefry_login(const char *UserID,const char *User_pass),
		Nefry_Auth(const char *Nefryclass,const char *NefryID), 
		memory_read_mode(const int num);
	
	bool push_SW(),
		memory_write_mode(const int pt,const int num),
		memory_write(const char *pt,const int num);
	char* memory_read(const int num);
	String read();
	//void webpage(const char url[20],String page,String link);

protected:
	ESP8266WebServer nefry_server;

private:
	bool push_sw_();
	void nefry_console();
	void module_set();
	void printWiFiConf(void);
	bool loadConf();
	void saveConf(void);
	void setDefaultModuleId(char* dst);
	void resetModule(void);
	void scanWiFi(void);
	int  waitConnected(void);
	void printIP(void);
	void setupWiFiConf(void);
	void setupWeb_local_Update(void);
	void setupWeb(void);
	String escapeParameter(String param);
	void setConf(char *old, const char *newdata);
	//void user_webpage();

};
extern Nefry_lib Nefry;
#endif
