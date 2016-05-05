#ifndef Nefry_h
#define Nefry_h

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
	String network_html,network_list, input_console;
	void
		reset(),
		sleep(int sec),
		Nefry_LED_begin(int num, int pin, uint8_t t),
		Nefry_LED(char r, char g, char b, char w = 122, char pin = 0, int num = 0),
		Nefry_LED_blink(char r, char g, char b, int wait, int loop, char pin = 0),
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
		nefry_loop();
	
	int available();
	String read();

	bool push_SW(),
		memory_write(const char *pt, int num);
	char* memory_read(int num);
	//void webpage(const char url[20],String page,String link);
	
protected:
    ESP8266WebServer _server;
	Adafruit_NeoPixel _RGBLED;

private:
	void nefry_console();
    void printWiFiConf(void);
    bool loadConf();
    void saveConf(void);
    void setDefaultModuleId(char* dst);
    void resetModuleId(void);
    void scanWiFi(void);
    int  waitConnected(void);
    void printIP(void);
    void setupWiFiConf(void);
    void setupWeb_local_Update(void);
    void setupWeb(void);
    String escapeParameter(String param);
    //void user_webpage();
 
};
extern Nefry_lib Nefry;
#endif
