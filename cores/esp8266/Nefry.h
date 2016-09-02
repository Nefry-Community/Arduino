#ifndef Nefry_h
#define Nefry_h
#include <Esp.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266httpUpdate.h>	
#include <DNSServer.h>
//#include <FS.h>

#define WIFI_CONF_FORMAT {37, 0, 0, 1}
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

	void
		reset(),
		sleep(const int sec),
		setProgramName(const char * pn),
		setConfHtmlPrint(const bool data, const int num),
		setConfWifi(const char SSID[32], const char pass[64]),
		setConfModule(const char module_id_[32], const char module_class_[32], const char module_wifi_pass_[64]),
		setConfUser(const char user[32], const char pass[32]),
		setConfHtml(const char set[15], const int num),
		beginLed(const int num, const int pin, uint8_t t),
		setLed(const char r, char g, const char b, const char w = 122, const char pin = 0, const int num = 0),

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
		print(String text,int ln=0),
		println(String text),
		nefry_init(),
		nefry_loop(),
		ndelay(unsigned long ms),
		setWebUpdate(String program_domain,String program_url);

	int available(),
		getConfValue(const int num);

	bool push_SW(),
		autoConnect(int sec=2),
		getConfHtmlPrint(const int num),
		setConfValue(const int pt, const int num),
		setConfStr(const char *pt, const int num),
		login(const char *UserID, const char *User_pass),
		Auth(const char *Nefryclass, const char *NefryID);

	char* getConfStr(const int num);
		

	String read(),
		getVersion(),
		getProgramName();
	char* getModuleName();
	//void webpage(const char url[20],String page,String link);

protected:
	ESP8266WebServer nefry_server;
	DNSServer _dnsServer;
private:
	void cssAdd(const char* id, String data, bool afterflg = 1);
	bool checkWebVersionFile();
	void downloadWebFile();
	void spiffsWeb(const char *fname, String stradd = "");
	String network_html, network_list, input_console;
	void Nefry_LED_blink(const char r, const char g, const char b, const int wait, const int loop, const char pin = 0);
	char module_input[20][15];
	bool push_sw_();
	void module_set();
	bool loadConf();
	void saveConf(void);
	void setDefaultModuleId(char* dst);
	void resetModule(void);
	void scanWiFi(void);
	int  waitConnected(void);
	void printModule(void);

	void setupWeb(void),
		setupWebModuleConf(void),
		setupWebLocalUpdate(void),
		setupWebOnlineUpdate(void),
		setupWebConsole(void),
		setupWebCaptivePortal(void),
		setupWebMain(void),
		setupWebCss(void),
		setupWebWiFiConf(void),
		setupModule(void),
		setupWifi(void);
	String escapeParameter(String param);
	void setConf(char *old, const char *newdata); 
	void printIpaddress();

};
extern Nefry_lib Nefry;
#endif
