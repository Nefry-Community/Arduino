/**
Nefry lib

Copyright (c) 2015 wami

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/

#include "Nefry.h"
const uint8_t wifi_conf_format[] = WIFI_CONF_FORMAT;
struct WiFiConfStruct {
	uint8_t format[4];
	char sta_ssid[32];
	char sta_pwd[64];
	char module_id[32];
	char module_class[32];
	char module_wifi_pwd[64];
	char Nefry_user[32];
	char Nefry_user_pass[32];
	char memo1[128];
	char memo2[128];
	char memo3[128];
	char memo4[64];
	char memo5[64];
	char memo6[64];
	char memo7[64];
	char memo8[64];
	int mode[8];
} WiFiConf = {
	WIFI_CONF_FORMAT,
	"Nefry",
	"Nefry-wifi",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	0
};
Adafruit_NeoPixel _NefryLED[17];
String Nefry_lib::getVersion()
{
	return "02";
}
bool Nefry_lib::push_SW() {
	pinMode(4, INPUT_PULLUP);
	if (digitalRead(4) == LOW)return true;
	return false;
}
bool Nefry_lib::push_sw_() {
	pinMode(4, INPUT_PULLUP);
	if (digitalRead(4) == LOW) {
		pushSW_flg = 1;
		setLed(0xff, 0x00, 0x00);
		delay(1000);
		return true;
	}
	return false;
}
void Nefry_lib::module_set() {
	for (int i = 0; i < 8; i++) {
		sprintf(module_input[i], "memo %d", i);
	}
	for (int i = 10; i < 18; i++) {
		sprintf(module_input[i], "mode %d", i - 10);
	}
}
void Nefry_lib::ndelay(unsigned long ms) {
	unsigned long beforetime = millis();
	while (millis() - beforetime < ms) {
		if (millis() < beforetime)break;
		nefry_loop();
		delay(1);
		//esp_yield();
	}
}
void Nefry_lib::setConfHtml(const char set[15], const int num) {
	if (0 <= num&&num < 20)
		strcpy(module_input[num], set);
}
void Nefry_lib::nefry_init() {
	ESP.wdtDisable();
	ESP.wdtEnable(60000);
	pinMode(4, INPUT_PULLUP);
	push_sw_();
	module_set();
	beginLed(1, 0, NEO_RGB + NEO_KHZ800);
	nefry_server = ESP8266WebServer(80);
	setLed(0x00, 0x0f, 0x00);
	Serial.begin(115200);
	EEPROM.begin(1024);
	delay(10);
	Serial.println("\n\nStartup");
	push_sw_();
	if (!loadConf()) {
		resetModule();
		saveConf();
	}
	setLed(0x00, 0x2f, 0x00);
	// scan Access Points
	scanWiFi();
	push_sw_();
	// start WiFi
	WiFi.mode(WIFI_AP_STA);
	WiFi.begin(WiFiConf.sta_ssid, WiFiConf.sta_pwd);
	waitConnected();
	if (WiFi.status() != WL_CONNECTED) {
		Nefry_LED_blink(0x00, 0x0, 0xFF, 100, 10);
		WiFi.mode(WIFI_AP);
	}
	String module_pass = WiFiConf.module_wifi_pwd;
	if (module_pass.length() == 0) {
		WiFi.softAP(WiFiConf.module_id);
	}
	else {
		WiFi.softAP(WiFiConf.module_id, WiFiConf.module_wifi_pwd);
	}
	setLed(0x00, 0x4f, 0x00);
	printIP();
	push_sw_();
	// setup Web Interface
	CaptivePortal();
	nefry_server.begin();
	setupWeb();
	nefry_console();
	setupWiFiConf();
	setupWeb_local_Update();
	//user_webpage();
	Serial.println("\nServer started");
	setLed(0x00, 0x6f, 0x00);
	// start mDNS responder
	if (!MDNS.begin("nefry")) {
		Serial.println("Error setting up MDNS responder!");
		setLed(0xff, 0x0, 0x00);
		delay(1000);
	}
	Serial.println("\nmDNS responder started");
	// Add service to MDNS-SD
	MDNS.addService("http", "tcp", 80);
	push_sw_();
	setLed(0x00, 0xff, 0xff);
	delay(1000);
	if (pushSW_flg) {
		WiFi.softAP(WiFiConf.module_id);
	}
}
void Nefry_lib::printWiFiConf(void) {
	Serial.print("SSID: ");
	Serial.println(WiFiConf.sta_ssid);
	Serial.print("Module ID: ");
	Serial.println(WiFiConf.module_id);
}

void Nefry_lib::setDefaultModuleId(char* dst) {
	uint8_t macAddr[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(macAddr);
	sprintf(dst, "%s%02x%02x", NAME_PREF, macAddr[WL_MAC_ADDR_LENGTH - 2], macAddr[WL_MAC_ADDR_LENGTH - 1]);
}
void Nefry_lib::resetModule(void) {
	uint8_t macAddr[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(macAddr);
	setDefaultModuleId(WiFiConf.module_id);
	Serial.print("Reset Module ID to default: ");
	Serial.println(WiFiConf.module_id);
}
void Nefry_lib::scanWiFi(void) {
	int founds = WiFi.scanNetworks();
	Serial.println();
	Serial.println("scan done");
	if (founds == 0) {
		Serial.println("no networks found");
	}
	else {
		Serial.print(founds);
		Serial.println(" networks found");
		for (int i = 0; i < founds; ++i) {
			// Print SSID and RSSI for each network found
			Serial.print(i + 1);
			Serial.print(": ");
			Serial.print(WiFi.SSID(i));
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));
			Serial.print(")");
			Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
			delay(10);
		}
	}
	Serial.println();
	network_html = "<ol>";
	network_list = "<datalist id=\"network_list\">";
	for (int i = 0; i < founds; ++i) {
		// Print SSID and RSSI for each network found
		network_html += "<li>";
		network_html += WiFi.SSID(i);
		network_html += " (";
		network_html += WiFi.RSSI(i);
		network_html += ")";
		network_html += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
		network_html += "</li>";
		network_list += "<option value=\"";
		network_list += WiFi.SSID(i);
		network_list += "\"></option>";
	}
	network_html += "</ol>";
	network_list += "</datalist>";
}
int Nefry_lib::waitConnected(void) {
	int wait = 0;
	Serial.println();
	Serial.println("Waiting for WiFi to connect");
	while (wait < 28) {
		if (WiFi.status() == WL_CONNECTED) {
			Serial.println("");
			Serial.println("WiFi connected");
			return (1);
			ESP.wdtFeed();
		}
		Nefry_LED_blink(0x00, 0x4f, 0x00, 200, 1);
		Serial.print(".");
		wait++;
		delay(250);
	}
	Serial.println("");
	Serial.println("Connect timed out");
	return (0);
}
void Nefry_lib::printIP(void) {
	Serial.println();
	Serial.print("Name: ");
	Serial.print(WiFiConf.module_id);
	Serial.println(".local");
	Serial.print("LAN: ");
	Serial.println(WiFi.localIP());
	Serial.print("AP: ");
	Serial.println(WiFi.softAPIP());
}
const char css[] = "<style>*,:after,:before{box-sizing:border-box}body{margin:0;padding:20px;font-family:\"Noto Sans\",HelveticaNeue,\"Helvetica Neue\",Helvetica,Arial,sans-serif;line-height:1.65;color:#555}ol,ul{padding-left:20px}a{color:#54AFBA}a:hover{text-decoration:none}div{margin:0 auto;padding:20px 24px;background-color:#fff;box-shadow:0 0 1px 1px rgba(0,0,0,.1);border-radius:4px;max-width:600px}div>h1{font-size:1.4em;padding-bottom:4px;border-bottom:1px solid #efefef;margin-top:0;margin-bottom:20px}</style>";
void Nefry_lib::setupWiFiConf(void) {
	nefry_server.on("/wifi_conf", [&]() {
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Wifi Set</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\"><style>.row>label{width:50px}</style>";
		content += "</head><body><div><h1>Nefry Wifi Set</h1>";
		content += network_list;
		content += "<form method='get' action='set_wifi_conf'><div class=\"row\"> <label for=\"ssid\">SSID: </label> <div> <input name=\"ssid\" id=\"ssid\" maxlength=\"32\"list=\"network_list\" value=\"\"> </div></div>";
		content += "<div class=\"row\"> <label for=\"pwd\">PASS: </label> <div> <input type=\"password\" name=\"pwd\" id=\"pwd\"maxlength=\"64\"> </div></div>";
		content += "<div class=\"footer\"> <input type=\"submit\" value=\"Save\" onclick=\"return confirm(&quot;Are you sure you want to change the WiFi settings?&quot;);\"> </div></form><a href=\"/\">Back to top</a></div><div>";
		content += network_html;
		content += "</div></body></html>";
		nefry_server.send(200, "text/html", content);
	});

	nefry_server.on("/set_wifi_conf", [&]() {
		String new_ssid = nefry_server.arg("ssid");
		String new_pwd = nefry_server.arg("pwd");
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Wifi Set</title>";
		content += css;
		content += "</head><body><div><h1>Nefry Wifi Set</h1>";
		if (new_ssid.length() > 0) {
			new_ssid = escapeParameter(new_ssid);
			new_pwd = escapeParameter(new_pwd);

			new_ssid.toCharArray(WiFiConf.sta_ssid, sizeof(WiFiConf.sta_ssid));
			new_pwd.toCharArray(WiFiConf.sta_pwd, sizeof(WiFiConf.sta_pwd));
			saveConf();
			content += "<p>saved '";
			content += WiFiConf.sta_ssid;
			content += "'... Restart to boot into new WiFi</p>";
			content += "<a href=\"/\">Back to top</a></div><body></html>";
			nefry_server.send(200, "text/html", content);
			delay(1000);
			ESP.restart();
		}
		else {
			content += "<p>Empty SSID is not acceptable. </p>";
			content += "<a href=\"/\">Back to top</a></div><body></html>";
			Serial.println("Rejected empty SSID.");
			nefry_server.send(200, "text/html", content);
		}
	});

	nefry_server.on("/module_id", [&]() {
		char defaultId[sizeof(WiFiConf.module_id)];
		setDefaultModuleId(defaultId);
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Module ID</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		content += "</head><body><div><h1>Nefry Module Setup</h1>";
		content += "<p>Module ID: ";
		content += WiFiConf.module_id;
		content += "</p>";
		content += "<form method='get' action='set_module_id'><div class=\"row\"> <label for=\"module_id\">New Module ID: </label> <div> <input name=\"module_id\" id=\"module_id\" maxlength=\"32\" value=\"";
		content += WiFiConf.module_id;
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">Module class: </label> <div> <input name=\"module_class\" id=\"module_class\" maxlength=\"32\" value=\"";
		content += WiFiConf.module_class;
		content += "\"></div></div>";
		content += "<div class=\"row\"> <label for=\"pwd\">Nefry wifi Pass: </label> <div> <input type=\"password\" name=\"pwd\" id=\"pwd\"maxlength=\"64\"> </div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">Nefry User: </label> <div> <input name=\"user\" id=\"user\" maxlength=\"32\" value=\"";
		content += WiFiConf.Nefry_user;
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">Nefry User Pass: </label> <div> <input name=\"user_pass\" id=\"user_pass\" maxlength=\"32\" value=\"\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[0];
		content += "</label> <div> <input name=\"memo1\" id=\"memo1\" maxlength=\"128\" value=\"";
		content += WiFiConf.memo1;
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[1];
		content += "</label> <div> <input name=\"memo2\" id=\"memo2\" maxlength=\"128\" value=\"";
		content += WiFiConf.memo2;
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[2];
		content += "</label> <div> <input name=\"memo3\" id=\"memo3\" maxlength=\"128\" value=\"";
		content += WiFiConf.memo3;
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[3];
		content += "</label> <div> <input name=\"memo4\" id=\"memo4\" maxlength=\"64\" value=\"";
		content += WiFiConf.memo4;
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[4];
		content += "</label> <div> <input name=\"memo5\" id=\"memo5\" maxlength=\"64\" value=\"";
		content += WiFiConf.memo5;
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[5];
		content += "</label> <div> <input name=\"memo6\" id=\"memo6\" maxlength=\"64\" value=\"";
		content += WiFiConf.memo6;
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[6];
		content += "</label> <div> <input name=\"memo7\" id=\"memo7\" maxlength=\"64\" value=\"";
		content += WiFiConf.memo7;
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[7];
		content += "</label> <div> <input name=\"memo8\" id=\"memo8\" maxlength=\"64\" value=\"";
		content += WiFiConf.memo8;
		content += "\"></div></div>";

		content += "<div class = \"footer\"><input type='submit' value=\"Save\"onclick='return confirm(&quot;Are you sure you want to change the Module ID?&quot;);'></div></form>";
		content += "<form method = 'get' action = 'reset'><div class = \"footer\"><input type='submit' value=\"Restart\"></div></form>";
		content += " <p>Empty will reset to default ID '";
		content += defaultId;
		content += "'</p><a href=\"/module_id_next\">Next page</a></br><a href=\"/\">Back to top</a></div></body></html>";
		nefry_server.send(200, "text/html", content);
	});

	nefry_server.on("/set_module_id", [&]() {
		String new_id = nefry_server.arg("module_id");
		String new_pwd = nefry_server.arg("pwd");
		String new_class = nefry_server.arg("module_class");
		String new_user = nefry_server.arg("user");
		String new_user_pass = nefry_server.arg("user_pass");
		String memo1 = nefry_server.arg("memo1");
		String memo2 = nefry_server.arg("memo2");
		String memo3 = nefry_server.arg("memo3");
		String memo4 = nefry_server.arg("memo4");
		String memo5 = nefry_server.arg("memo5");
		String memo6 = nefry_server.arg("memo6");
		String memo7 = nefry_server.arg("memo7");
		String memo8 = nefry_server.arg("memo8");

		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Module Set</title>";
		content += css;
		content += "</head><body><div><h1>Nefry Module Set</h1>";
		if (new_id.length() > 0)
			new_id.toCharArray(WiFiConf.module_id, sizeof(WiFiConf.module_id));
		else
			resetModule();
		if (new_pwd.length() > 0) {
			new_pwd.toCharArray(WiFiConf.module_wifi_pwd, sizeof(WiFiConf.module_wifi_pwd));
		}
		if (new_user_pass.length() > 0) {
			new_user_pass.toCharArray(WiFiConf.Nefry_user_pass, sizeof(WiFiConf.Nefry_user_pass));
		}
		new_user.toCharArray(WiFiConf.Nefry_user, sizeof(WiFiConf.Nefry_user));
		new_class.toCharArray(WiFiConf.module_class, sizeof(WiFiConf.module_class));
		memo1.toCharArray(WiFiConf.memo1, sizeof(WiFiConf.memo1));
		memo2.toCharArray(WiFiConf.memo2, sizeof(WiFiConf.memo2));
		memo3.toCharArray(WiFiConf.memo3, sizeof(WiFiConf.memo3));
		memo4.toCharArray(WiFiConf.memo4, sizeof(WiFiConf.memo4));
		memo5.toCharArray(WiFiConf.memo5, sizeof(WiFiConf.memo5));
		memo6.toCharArray(WiFiConf.memo6, sizeof(WiFiConf.memo6));
		memo7.toCharArray(WiFiConf.memo7, sizeof(WiFiConf.memo7));
		memo8.toCharArray(WiFiConf.memo8, sizeof(WiFiConf.memo8));

		saveConf();
		content += "<p>Set Module ID to '";
		content += WiFiConf.module_id;
		content += "' ... Restart. </p>";
		content += "<a href=\"/\">Back to top</a></div></body></html>";
		nefry_server.send(200, "text/html", content);
		delay(2000);
		ESP.restart();
	});
	nefry_server.on("/web_update", [&]() {
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\"><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		content += "<title>Nefry Web Update</title><style>.row>input,input[type=file]{width:100%}.row label{line-height:1.3;display:block;margin-bottom:5px;width:300px}.row>input{display:inline-block}</style>";
		content += "</head><body><div><h1>Nefry Web Update</h1>";
		content += "<form method='get' action='program'><div class=\"row\"> <label for=\"File\">Program download Domain: </label><input name='domain'id='URL' value=''><label for='File'>Program download URL(Domain except): </label><input name='URL'id='URL'  value=''placeholder=\"Null ok\">";
		content += "</div><div class=\"footer\"> <input type=\"submit\" value=\"Save\" onclick=\"\"></div></form></br><p>Default Program Download URL : wamisnet.github.io </p><a href='/'>Back to top</a></div>";
		content += "</body></html>";
		nefry_server.send(200, "text/html", content);
	});
	nefry_server.on("/program", [&]() {
		String program_domain = nefry_server.arg("domain");
		String program_url = nefry_server.arg("URL");
		nefry_server.sendHeader("Connection", "close");
		nefry_server.sendHeader("Access-Control-Allow-Origin", "*");
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Web Update</title>";// < link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		content += css;
		content += "</head><body><div><h1>Nefry Web Update</h1><p>";
		if (program_domain.length() > 0) {
			if (program_url.length() > 0)program_url = escapeParameter(program_url);
			print(program_url);
			program_url.concat("/arduino.bin");
			switch (ESPhttpUpdate.update(program_domain, 80, program_url)) {
			case HTTP_UPDATE_FAILED:
				Serial.println(program_url);
				Serial.println("[update] Update failed.");
				content += "[update] Update failed.";
				break;
			case HTTP_UPDATE_NO_UPDATES:
				Serial.println("[update] Update no Update.");
				content += "[update] Update no Update.";
				break;
			case HTTP_UPDATE_OK:
				Nefry_LED_blink(0x00, 0xff, 0xff, 250, 10);
				Serial.println("[update] Update ok.");
				content += "[update] Update ok.";// may not called we reboot the ESP
				content += "</br></p><a href='/'>Back to top</a></div><body></html>";
				nefry_server.send(200, "text/html", content);
				setLed(0x00, 0xff, 0xff);
				for (int i = 0; i < 20; i++) {
					delay(100);
					nefry_loop();
				}
				ESP.restart();
				break;
			}
		}
		else {
			content += "<p>Empty URL is not acceptable.</p>";
			Serial.println("Rejected empty URL.");
		}
		content += "</p><a href='/'>Back to top</a></div><body></html>";
		nefry_server.send(200, "text/html", content);
	});

	nefry_server.on("/module_id_next", [&]() {

		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Module ID</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		content += "</head><body><div><h1>Nefry Module Setup</h1>";
		content += "<p>Module ID: ";
		content += WiFiConf.module_id;
		content += "</p>";
		content += "<form method='get' action='set_module_id_next'>";

		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[10];
		content += "</label> <div> <input name=\"mode0\" id=\"mode0\" type=\"number\" value=\"";
		content += WiFiConf.mode[0];
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[11];
		content += "</label> <div> <input name=\"mode1\" id=\"mode1\" type=\"number\" value=\"";
		content += WiFiConf.mode[1];
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[12];
		content += "</label> <div> <input name=\"mode2\" id=\"mode2\" type=\"number\" value=\"";
		content += WiFiConf.mode[2];
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[13];
		content += "</label> <div> <input name=\"mode3\" id=\"mode3\" type=\"number\" value=\"";
		content += WiFiConf.mode[3];
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[14];
		content += "</label> <div> <input name=\"mode4\" id=\"mode4\" type=\"number\" value=\"";
		content += WiFiConf.mode[4];
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[15];
		content += "</label> <div> <input name=\"mode5\" id=\"mode5\" type=\"number\" value=\"";
		content += WiFiConf.mode[5];
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[16];
		content += "</label> <div> <input name=\"mode6\" id=\"mode6\" type=\"number\" value=\"";
		content += WiFiConf.mode[6];
		content += "\"></div></div>";
		content += "<div class = \"row\"> <label for=\"module_id\">";
		content += module_input[17];
		content += "</label> <div> <input name=\"mode7\" id=\"mode7\" type=\"number\" value=\"";
		content += WiFiConf.mode[7];
		content += "\"></div></div>";
		content += "<div class = \"footer\"><input type='submit' value=\"Save\"onclick='return confirm(&quot;Are you sure you want to change the Module ID?&quot;);'></div></form>";
		content += "<form method = 'get' action = 'reset'><div class = \"footer\"><input type='submit' value=\"Restart\"></div></form>";
		content += "</br>macAddress : ";
		content += WiFi.macAddress();
		IPAddress ip = WiFi.localIP();
		String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
		content += "</br>IPAddress : ";
		content += ipStr;
		ip = WiFi.subnetMask();
		ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
		content += "</br>subNetMask : ";
		content += ipStr;
		ip = WiFi.gatewayIP();
		ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
		content += "</br>Gateway IPAddress : ";
		content += ipStr;
		content += "</br>";
		content += "<a href=\"/module_id\">Prev page</a></br><a href=\"/\">Back to top</a></div></body></html>";
		nefry_server.send(200, "text/html", content);
	});

	nefry_server.on("/set_module_id_next", [&]() {
		String mode0 = nefry_server.arg("mode0");
		String mode1 = nefry_server.arg("mode1");
		String mode2 = nefry_server.arg("mode2");
		String mode3 = nefry_server.arg("mode3");
		String mode4 = nefry_server.arg("mode4");
		String mode5 = nefry_server.arg("mode5");
		String mode6 = nefry_server.arg("mode6");
		String mode7 = nefry_server.arg("mode7");
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Module Set</title>";
		content += css;
		content += "</head><body><div><h1>Nefry Module Set</h1>";

		WiFiConf.mode[0] = mode0.toInt();
		WiFiConf.mode[1] = mode1.toInt();
		WiFiConf.mode[2] = mode2.toInt();
		WiFiConf.mode[3] = mode3.toInt();
		WiFiConf.mode[4] = mode4.toInt();
		WiFiConf.mode[5] = mode5.toInt();
		WiFiConf.mode[6] = mode6.toInt();
		WiFiConf.mode[7] = mode7.toInt();
		saveConf();
		content += "<p>Set Module ID to '";
		content += WiFiConf.module_id;
		content += "' ... Restart. </p>";
		content += "<a href=\"/\">Back to top</a></div></body></html>";
		nefry_server.send(200, "text/html", content);
		delay(2000);
		ESP.restart();
	});
	nefry_server.on("/reset", [&]() {
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Reset</title>";
		content += css;
		content += "</head><body><div><h1>Nefry Reset</h1>";
		content += "<p>Reset start!</p>";
		content += "<a href=\"/\">Back to top</a></div></body></html>";
		nefry_server.send(200, "text/html", content);
		delay(2000);
		ESP.restart();
	});

}
void Nefry_lib::setupWeb_local_Update(void) {
	int count = 0;
	nefry_server.on("/update", HTTP_GET, [&]() {
		nefry_server.sendHeader("Connection", "close");
		nefry_server.sendHeader("Access-Control-Allow-Origin", "*");
		String sketchUploadForm = "<!DOCTYPE HTML><head><meta charset=\"UTF-8\">";
		sketchUploadForm += "<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		sketchUploadForm += "<title>Nefry Upload Sketch</title></head><body><div><h1>Upload Sketch</h1> <p>Upload a binary file of sketch.</p><form method=\"POST\" action=\"/upload_sketch\" enctype=\"multipart/form-data\">";
		sketchUploadForm += "<input type=\"file\" name=\"sketch\"><div class=\"footer\"> <input type=\"submit\" value=\"Upload\" onclick=\"return confirm(&quot;Are you sure you want to update the Sketch?&quot;);\">";
		sketchUploadForm += "</div></form><a href=\"/\">Back to top</a></div></body></html>";
		nefry_server.send(200, "text/html", sketchUploadForm);
	});
	nefry_server.onFileUpload([&]() {
		if (nefry_server.uri() != "/upload_sketch") return;
		HTTPUpload& upload = nefry_server.upload();
		Serial.printf("Sketch: %s Uploading\n", upload.filename.c_str());
		String file_name = String(upload.filename.c_str());
		count++;
		if (file_name.endsWith("bin")) {
			if (count % 10 == 1)
				Nefry_LED_blink(0x00, 0x0, 0xFF, 30, 2);
			if (upload.status == UPLOAD_FILE_START) {
				Serial.println("ok");
				Serial.setDebugOutput(true);
				WiFiUDP::stopAll();
				uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
				if (!Update.begin(maxSketchSpace))Update.printError(Serial);
			}
			else if (upload.status == UPLOAD_FILE_WRITE) {
				if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)Update.printError(Serial);
			}
			else if (upload.status == UPLOAD_FILE_END) {
				if (Update.end(true)) Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
				else Update.printError(Serial);
				Serial.setDebugOutput(false);
			}
			yield();
		}
		else {
			if (count % 15 == 1)
				Nefry_LED_blink(0xFF, 0x0, 0x00, 80, 1);
			Serial.println("err");
			nefry_server.on("/upload_sketch", HTTP_POST, [&]() {
				nefry_server.sendHeader("Connection", "close");
				nefry_server.sendHeader("Access-Control-Allow-Origin", "*");
				String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
				content += "<title>Nefry Upload Sketch</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
				content += "</head><body><div><h1>Nefry Upload Err</h1>";
				content += "<p>File Err</p>";
				content += "<a href='/'>Back to top</a></div></body></html>";
				nefry_server.send(200, "text/html", content);
			});
			return;
		}
	});
	nefry_server.on("/upload_sketch", HTTP_POST, [&]() {
		nefry_server.sendHeader("Connection", "close");
		nefry_server.sendHeader("Access-Control-Allow-Origin", "*");
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Upload Sketch</title>";
		content += css;
		content += "</head><body><div><h1>Nefry Update</h1><p>";
		content += (Update.hasError()) ? "Update Err" : "Upload Success";
		content += "</p><a href='/'>Back to top</a></div></body></html>";
		nefry_server.send(200, "text/html", content);
		delay(2000);
		ESP.restart();
	});
}
void Nefry_lib::setupWeb(void) {
	nefry_server.on("/", [&]() {
		IPAddress ip = WiFi.localIP();
		String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Menu</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		content += "</head><body><div><h1>Hello from Nefry!</h1>";
		content += "<p>Wifi Sport: ";
		content += WiFiConf.sta_ssid;
		content += "</br>IP Address: ";
		content += ipStr;
		content += "</br>Module ID: ";
		content += WiFiConf.module_id;
		content += "</p>";
		content += "<ul>";
		content += "<li><a href='/wifi_conf'>Setup WiFi</a>";
		content += "<li><a href='/module_id'>Setup Module</a>";
		content += "<li><a href='/web_update'>Web Sketch Download</a>";
		content += "<li><a href='/update'>Upload Sketch</a>";
		content += "<li><a href='/console'>Web Console</a>";
		//if(User_page.length() > 0) {
		//	content += "<li><a href='/user'>Create Page</a>";
		//}
		content += "</ul>";
		content += "</div></body></html>";
		nefry_server.send(200, "text/html", content);
	});
	nefry_server.on("/nefry_css", [&]() {
		char content[] = "@charset \"UTF-8\"; *, :after, : before{ box - sizing:border - box }body{ font:16px / 1.65 \"Noto Sans\",\"HelveticaNeue\",\"Helvetica Neue\",Helvetica,Arial,sans-serif;margin:0;padding:0 20px;color:#555}ol,ul{padding-left:20px;margin:0}a{color:#54AFBA}a:hover{text-decoration:none}body>div{background:#fff;margin:20px auto;padding:20px 24px;box-shadow:0 0 1px 1px rgba(0,0,0,.1);border-radius:4px;max-width:540px}body>div input,body>div li{word-wrap:break-word}body>div>h1{font-size:1.4em;line-height:1.3;padding-bottom:4px;border-bottom:1px solid #efefef;margin-top:0;margin-bottom:20px}input,select,textarea{font:inherit inherit inherit}input{background:rgba(0,0,0,0);padding:.4em .6em;border:1px solid rgba(0,0,0,.12);border-radius:3px;-webkit-appearance:none;-moz-appearance:none;appearance:none}input:focus{border:1px solid #6E5F57;outline:0}input[type=submit]{margin-left:6px;cursor:pointer;line-height:2.6;display:inline-block;padding:0 1.2rem;text-align:center;vertical-align:middle;color:#FFF;border:0;border-radius:3px;background:#6E5F57;-webkit-appearance:none;-moz-appearance:none;appearance:none}.row,.row>div,.row>label{display:block}input[type=submit]:hover{color:#FFF;background:#372F2A}input[type=submit]:focus{outline:0}input[type=file]{width:100%}.row{margin-bottom:14px}.row>label{float:left;width:110px;font-size:14px;position:relative;top:8px}.row>div{margin-left:120px;margin-bottom:12px}.row>div>input{width:100%;display:inline-block}.footer{text-align:right;margin-top:16px}";
		nefry_server.send(200, "text/css", content);
	});
}
void Nefry_lib::nefry_loop() {
	_dnsServer.processNextRequest();
	nefry_server.handleClient();
}
String Nefry_lib::escapeParameter(String param) {
	param.replace("+", " ");
	param.replace("%21", "!");
	param.replace("%22", "\"");
	param.replace("%23", "#");
	param.replace("%24", "$");
	param.replace("%25", "%");
	param.replace("%26", "&");
	param.replace("%27", "'");
	param.replace("%28", "(");
	param.replace("%29", ")");
	param.replace("%2A", "*");
	param.replace("%2B", "+");
	param.replace("%2C", ",");
	param.replace("%2F", "/");
	param.replace("%3A", ":");
	param.replace("%3B", ";");
	param.replace("%3C", "<");
	param.replace("%3D", "=");
	param.replace("%3E", ">");
	param.replace("%3F", "?");
	param.replace("%40", "@");
	param.replace("%5B", "[");
	param.replace("%5C", "\\");
	param.replace("%5D", "]");
	param.replace("%5E", "^");
	param.replace("%60", "'");
	param.replace("%7B", "{");
	param.replace("%7C", "|");
	param.replace("%7D", "}");
	return param;
}

void Nefry_lib::println(float text) { println(String(text)); }
void Nefry_lib::println(double text) { println(String(text)); }
void Nefry_lib::println(char text) { println(String(text)); }
void Nefry_lib::println(int text) { println(String(text)); }
void Nefry_lib::println(long text) { println(String(text)); }
void Nefry_lib::println(unsigned char text) { println(String(text)); }
void Nefry_lib::println(unsigned int text) { println(String(text)); }
void Nefry_lib::println(unsigned long text) { println(String(text)); }
void Nefry_lib::print(float text) { print(String(text)); }
void Nefry_lib::print(double text) { print(String(text)); }
void Nefry_lib::print(char text) { print(String(text)); }
void Nefry_lib::print(int text) { print(String(text)); }
void Nefry_lib::print(long text) { print(String(text)); }
void Nefry_lib::print(unsigned char text) { print(String(text)); }
void Nefry_lib::print(unsigned int text) { print(String(text)); }
void Nefry_lib::print(unsigned long text) { print(String(text)); }
int printcun;
#define max_console 30
char printweb[max_console][60], moji[60];
int mojicount = 0;
void Nefry_lib::print(String text) {
	if (printcun >= max_console) {
		Serial.println("over");
		printcun = 0;
	}
	Serial.println(text);
	text.toCharArray(moji, 60);
	strcpy(printweb[printcun], moji);
	printcun++;
	if (mojicount < max_console)
		mojicount++;

}
void Nefry_lib::println(String text) {
	print(text + "<br>");
}

void Nefry_lib::nefry_console() {
	nefry_server.on("/console", [&]() {
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Console</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		content += "";
		content += "<script type = \"text/javascript\">\n";
		content += "	function loadDoc() {\n";
		content += "	if (window.XMLHttpRequest) {\n";
		content += "		xmlhttp = new XMLHttpRequest();\n";
		content += "	} else {\n";
		content += "		xmlhttp = new ActiveXObject(\"Microsoft.XMLHTTP\");\n";
		content += "	}\n";
		content += "	xmlhttp.onreadystatechange = function() {\n";
		content += "		if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {\n";
		content += "			document.getElementById(\"ajaxDiv\").innerHTML=xmlhttp.responseText;\n";
		content += "			console.log(\"get\");\n";
		content += "		}\n";
		content += "	}\n";
		content += "	xmlhttp.open(\"GET\",\"tout\",true);\n";
		content += "	xmlhttp.send();\n";
		content += "}\n";
		content += "var timer; \n";
		content += "timer = setInterval(\"loadDoc()\",2000);\n";
		content += "function reload(time) {\n";
		content += "	clearInterval(timer); \n";
		content += "	timer = setInterval(\"loadDoc()\",time);\n";
		content += "}\n";
		content += "</script>\n";
		content += "</head><body><div><h1>Nefry Console</h1>";
		content += "<p>It can be used as a terminal.</p>";
		content += "<form method='get' action='set_console'><div class=\"row\"> <label for=\"console\">console:</label> <div> <input name=\"console\" id=\"console\" maxlength=\"100\" value=\"\"> </div></div>";
		content += "<div class=\"footer\"> <input type=\"submit\" value=\"Send\"> </div></form>";
		content += "<p><div><div id=\"ajaxDiv\"></div><button type=\"button\" onclick=\"loadDoc()\">reload</button>";
		content += "<button type = \"button\" onclick=\"reload(500);\">0.5sec reload</button>";
		content += "<button type = \"button\" onclick=\"reload(2000);\">2sec reload</button>";
		content += "<button type = \"button\" onclick=\"reload(5000);\">5sec reload</button>";
		content += "<button type = \"button\" onclick=\"clearInterval(timer);\">stop</button>";
		content += "</div></p></br><a href=\"/\">Back to top</a></div>";
		content += "</body></html>";
		nefry_server.send(200, "text/html", content);
	});
	nefry_server.on("/tout", HTTP_GET, [&]() {
		String content;
		int i;
		i = printcun;
		if (mojicount < max_console)i = 0;
		for (int j = 0; j <= mojicount; j++, i++) {
			if (i > max_console)i = 0;
			content += printweb[i];
		}
		nefry_server.send(200, "text/html", content);
	});

	nefry_server.on("/set_console", [&]() {
		input_console = nefry_server.arg("console");
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\"><meta http-equiv=\"Refresh\" content=\"3; URL = /console\">";
		content += "<title>Nefry Console Send</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		content += "</head><body><div><h1>Nefry Console Send</h1>";
		if (input_console.length() > 0) {
			input_console = escapeParameter(input_console);
			content += "<p>send  \" ";
			content += input_console;
			println(input_console);
			content += " \"  </p>";
			content += "<p>To return after 3 seconds</p>";
		}
		else content += "<p>Null data.</p></br>";
		content += "</br><a href=\"/\">Back to top</a></br><a href=\"/console\">Back to console</a></div>";
		content += "</body></html>";
		nefry_server.send(200, "text/html", content);
	});
}
int Nefry_lib::available() {
	return input_console.length();
}
String Nefry_lib::read() {
	String read_console = input_console;
	input_console = "";
	return read_console;
}

bool Nefry_lib::loadConf() {
	Serial.println();
	Serial.println("loading WiFiConf");
	if (EEPROM.read(WIFI_CONF_START + 0) == wifi_conf_format[0] &&
		EEPROM.read(WIFI_CONF_START + 1) == wifi_conf_format[1] &&
		EEPROM.read(WIFI_CONF_START + 2) == wifi_conf_format[2] &&
		EEPROM.read(WIFI_CONF_START + 3) == wifi_conf_format[3])
	{
		for (unsigned int t = 0; t < sizeof(WiFiConf); t++) {
			*((char*)&WiFiConf + t) = EEPROM.read(WIFI_CONF_START + t);
		}
		printWiFiConf();
		return true;
	}
	else {
		Serial.println("WiFiConf was not saved on EEPROM.");
		return false;
	}
}
void Nefry_lib::saveConf(void) {
	Serial.println("writing WiFiConf");
	for (unsigned int t = 0; t < sizeof(WiFiConf); t++) {
		EEPROM.write(WIFI_CONF_START + t, *((char*)&WiFiConf + t));
	}
	EEPROM.commit();
	//printWiFiConf();
}
bool Nefry_lib::setConfStr(const char *pt, int num) {
	Serial.println("memory write");
	switch (num + 1) {
	case 1:
		if (strcmp(WiFiConf.memo1, pt) != 0) {
			strcpy(WiFiConf.memo1, pt);
			saveConf();
		}
		break;
	case 2:
		if (strcmp(WiFiConf.memo2, pt) != 0) {
			strcpy(WiFiConf.memo2, pt);
			saveConf();
		}
		break;
	case 3:
		if (strcmp(WiFiConf.memo3, pt) != 0) {
			strcpy(WiFiConf.memo3, pt);
			saveConf();
		}
		break;
	case 4:
		if (strcmp(WiFiConf.memo4, pt) != 0) {
			strcpy(WiFiConf.memo4, pt);
			saveConf();
		}
		break;
	case 5:
		if (strcmp(WiFiConf.memo5, pt) != 0) {
			strcpy(WiFiConf.memo5, pt);
			saveConf();
		}
		break;
	case 6:
		if (strcmp(WiFiConf.memo6, pt) != 0) {
			strcpy(WiFiConf.memo6, pt);
			saveConf();
		}
		break;
	case 7:
		if (strcmp(WiFiConf.memo7, pt) != 0) {
			strcpy(WiFiConf.memo7, pt);
			saveConf();
		}
		break;
	case 8:
		if (strcmp(WiFiConf.memo8, pt) != 0) {
			strcpy(WiFiConf.memo8, pt);
			saveConf();
		}
		break;
	default:
		return false;
		break;
	}
	return true;
}

char* Nefry_lib::getConfStr(const int num) {
	Serial.println("memory read");
	switch (num + 1) {
	case 1:
		return WiFiConf.memo1;
		break;
	case 2:
		return WiFiConf.memo2;
		break;
	case 3:
		return WiFiConf.memo3;
		break;
	case 4:
		return WiFiConf.memo4;
		break;
	case 5:
		return WiFiConf.memo5;
		break;
	case 6:
		return WiFiConf.memo6;
		break;
	case 7:
		return WiFiConf.memo7;
		break;
	case 8:
		return WiFiConf.memo8;
		break;
	}
	return (char*)"";
}
bool Nefry_lib::setConfValue(const int pt, const int num) {
	if (0 <= num && num <= 7) {
		if (WiFiConf.mode[num] != pt) {
			WiFiConf.mode[num] = pt;
			saveConf();
			return true;
		}
		return false;
	}
	else
		return false;
}
int Nefry_lib::getConfValue(const int num) {
	Serial.println("memory read");
	if (0 <= num && num <= 7)
		return WiFiConf.mode[num];
	else
		return 0;
}

void Nefry_lib::setLed(const char r, const char g, const char b, const char w, const char pin, const int num) {
	_NefryLED[pin].setBrightness(w);
	_NefryLED[pin].setPixelColor(num, r, g, b);
	_NefryLED[pin].show();
}


void Nefry_lib::CaptivePortal() {
	_dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));

	nefry_server.onNotFound([&]() {
		IPAddress ip = WiFi.localIP();
		String responseHTML = ""
			"<!DOCTYPE html><html><head><title>CaptivePortal</title></head><meta http-equiv=\"Refresh\" content=\"1; URL = http://192.168.4.1\"><body>"
			"<h1>Move to main page!</h1><a href=\"http://192.168.4.1\">Move to Top page!</a>";
		"</body></html>";
		nefry_server.send(200, "text/html", responseHTML);
	});
}

void Nefry_lib::Nefry_LED_blink(const char r, const char g, const char b, const int wait, const int loop, const char pin) {
	int i = 0;
	while (i < loop) {
		setLed(r, g, b, 122, pin);
		delay(wait);
		setLed(r, g, b, 0, pin);
		delay(wait);
		i++;
	}
}
void Nefry_lib::beginLed(const int num, const int pin, uint8_t t = NEO_GRB + NEO_KHZ800) {
	_NefryLED[pin] = Adafruit_NeoPixel(num, pin, t);
	_NefryLED[pin].begin();
}

void Nefry_lib::reset() {
	ESP.restart();
}
void Nefry_lib::sleep(const int sec) {
	ESP.deepSleep(sec * 1000 * 1000, WAKE_RF_DEFAULT);
	delay(1000);
}
int Nefry_lib::login(const char *UserID, const char *User_pass) {
	if (strcmp(UserID, WiFiConf.Nefry_user) == 0)
		if (strcmp(User_pass, WiFiConf.Nefry_user_pass) == 0)
			return true;
	return false;
}
int Nefry_lib::Auth(const char *Nefryclass, const char *NefryID) {
	if (strcmp(NefryID, WiFiConf.module_id) == 0)return true;
	if (strcmp(Nefryclass, WiFiConf.module_class) == 0)return true;
	return false;
}
void Nefry_lib::setConf(char *old, const char *newdata) {
	if (strcmp(old, newdata) != 0) {
		strcpy(old, newdata);
		saveConf();
	}
}
void Nefry_lib::setConfWifi(const char SSID[32], const char pass[64]) {
	setConf(WiFiConf.sta_ssid, SSID);
	setConf(WiFiConf.sta_pwd, pass);
}
void Nefry_lib::setConfModule(const char module_id_[32], const char module_class_[32], const char module_wifi_pass[64]) {
	setConf(WiFiConf.module_class, module_class_);
	setConf(WiFiConf.module_wifi_pwd, module_wifi_pass);
	if (strlen(module_id_) == 0) {
		resetModule();
		saveConf();
	}
	else {
		setConf(WiFiConf.module_id, module_id_);
	}
}
void Nefry_lib::setConfUser(const char user[32], const char pass[32]) {
	setConf(WiFiConf.Nefry_user, user);
	setConf(WiFiConf.Nefry_user_pass, pass);
}
/*
void Nefry_lib::webpage(const char url[20],String page,String link){
	char uri[30]="/user/";
	char *u=uri;
	strcpy(u+6,url);
	print(page);
	/*nefry_server.on(url, [&]() {
		String pagee="<!DOCTYPE html><head><meta charset=\"UTF-8\"/><title>サイト名</title></head><body><h1>サイト名</h1></body></html>";
		nefry_server.send(200, "text/html", pagee);
	});*//*
		nefry_server.on(url, [&]() {
		IPAddress ip = WiFi.localIP();
		String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
		String content = "<!DOCTYPE HTML>\r\n<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1252\"><title>";
		content += WiFiConf.module_id;
		content += ".local";
		content += "</title><style>*{box-sizing: border-box;}*:before, *:after{box-sizing: border-box;}body{margin:0; padding: 20px;font-family: \"Noto Sans\",\"HelveticaNeue\",\"Helvetica Neue\",Helvetica,Arial,sans-serif; line-height: 1.65; color: #555;}ul, ol{padding-left: 20px}a{color: #54AFBA}a:hover{text-decoration: none}div{margin: 0 auto; padding: 20px 24px; background-color: #fff; box-shadow: 0 0 1px 1px rgba(0,0,0,.1); border-radius: 4px; max-width: 600px;}div > h1{font-size: 1.4em; padding-bottom: 4px; border-bottom: 1px solid #efefef; margin-top: 0; margin-bottom: 20px;}</style></head><body>";
		content += "<div><h1>Create</h1>";
		content += "<p>LAN: ";
		content += WiFiConf.sta_ssid;
		content += "</br>IP: ";
		content += ipStr;
		content += " ( ";
		content += WiFiConf.module_id;
		content += ".local";
		content += " )</p>";
		content += "<p>";
		content += "</p>";
		content += "<ul>";
		if(User_page.length() > 0) {
			content +=User_page;
		}else{
			content +="Null page";
		}
		content += "</ul>";
		content += "<a href=\"/\">Back to top</a></div></body></html>";
		nefry_server.send(200, "text/html", content);
	});
	User_page +="<li><a href=\"";
	User_page +=url;
	User_page +="\">";
	User_page +=link;
	User_page +="</a>";
}
void Nefry_lib::user_webpage(){
	nefry_server.on("/user", [&]() {
		IPAddress ip = WiFi.localIP();
		String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
		String content = "<!DOCTYPE HTML>\r\n<html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=windows-1252\"><title>";
		content += WiFiConf.module_id;
		content += ".local";
		content += "</title><style>*{box-sizing: border-box;}*:before, *:after{box-sizing: border-box;}body{margin:0; padding: 20px;font-family: \"Noto Sans\",\"HelveticaNeue\",\"Helvetica Neue\",Helvetica,Arial,sans-serif; line-height: 1.65; color: #555;}ul, ol{padding-left: 20px}a{color: #54AFBA}a:hover{text-decoration: none}div{margin: 0 auto; padding: 20px 24px; background-color: #fff; box-shadow: 0 0 1px 1px rgba(0,0,0,.1); border-radius: 4px; max-width: 600px;}div > h1{font-size: 1.4em; padding-bottom: 4px; border-bottom: 1px solid #efefef; margin-top: 0; margin-bottom: 20px;}</style></head><body>";
		content += "<div><h1>Create</h1>";
		content += "<p>LAN: ";
		content += WiFiConf.sta_ssid;
		content += "</br>IP: ";
		content += ipStr;
		content += " ( ";
		content += WiFiConf.module_id;
		content += ".local";
		content += " )</p>";
		content += "<p>";
		content += "</p>";
		content += "<ul>";
		if(User_page.length() > 0) {
			content +=User_page;
		}else{
			content +="Null page";
		}
		content += "</ul>";
		content += "<a href=\"/\">Back to top</a></div></body></html>";
		nefry_server.send(200, "text/html", content);
	});
	nefry_server.on("/ard", [&]() {
		nefry_server.send(200, "text/html","");
	});
}
*/
Nefry_lib Nefry;