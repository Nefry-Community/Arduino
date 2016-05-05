/**
Nefry lib

Copyright (c) 2016 wami

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
	char module_wifi_pwd[64];
	char memo1[128];
	char memo2[128];
	char memo3[128];
} WiFiConf = {
	WIFI_CONF_FORMAT,
	"Nefry",
	"Nefry-wifi",
	"",
	"",
	"",
	"",
	""
};
Adafruit_NeoPixel _NefryLED[17];

bool Nefry_lib::push_SW() {
	if (digitalRead(SW) == LOW) {
		pushSW_flg = 1;
		Nefry_LED(0xff, 0x00, 0x00);
		delay(1000);
		return 1;
	}
	return 0;
}
void Nefry_lib::nefry_init() {
	pinMode(SW, INPUT_PULLUP);
	push_SW();
	Nefry_LED_begin(1, 0, NEO_RGB + NEO_KHZ800);
	_server = ESP8266WebServer(80);
	Nefry_LED(0x00, 0x0f, 0x00);
	Serial.begin(115200);
	EEPROM.begin(580);
	delay(10);
	Serial.println("\n\nStartup");
	push_SW();
	if (!loadConf()) {
		resetModuleId();
		saveConf();
	}
	Nefry_LED(0x00, 0x2f, 0x00);
	// scan Access Points
	scanWiFi();
	push_SW();
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
	}else {
		WiFi.softAP(WiFiConf.module_id, WiFiConf.module_wifi_pwd);
	}
	Nefry_LED(0x00, 0x4f, 0x00);
	printIP();
	push_SW();
	// setup Web Interface

	_server.begin();
	setupWeb();
	nefry_console();
	setupWiFiConf();
	setupWeb_local_Update();
	//user_webpage();
	Serial.println("\nServer started");
	Nefry_LED(0x00, 0x6f, 0x00);
	// start mDNS responder
	if (!MDNS.begin("nefry")) {
		Serial.println("Error setting up MDNS responder!");
		Nefry_LED(0xff, 0x0, 0x00);
		delay(1000);
	}
	Serial.println("\nmDNS responder started");
	// Add service to MDNS-SD
	MDNS.addService("http", "tcp", 80);
	push_SW();
	Nefry_LED(0x00, 0xff, 0xff);
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
void Nefry_lib::resetModuleId(void) {
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
	}else {
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
	for (int i = 0; i < founds; ++i){
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
	while (wait < 20) {
		if (WiFi.status() == WL_CONNECTED) {
			Serial.println("");
			Serial.println("WiFi connected");
			return (1);
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
		_server.on("/wifi_conf", [&]() {
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Wifi Set</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\"><style>.row>label{width:50px}</style>";
		content += "</head><body><div><h1>Nefry Wifi Set</h1>";
		content += network_list;
		content += "<form method='get' action='set_wifi_conf'><div class=\"row\"> <label for=\"ssid\">SSID: </label> <div> <input name=\"ssid\" id=\"ssid\" maxlength=\"32\"list=\"network_list\" value=\"\"> </div></div>";
		content += "<div class=\"row\"> <label for=\"pwd\">PASS: </label> <div> <input type=\"password\" name=\"pwd\" id=\"pwd\"maxlength=\"64\"> </div></div>";
		content += "<div class=\"footer\"> <input type=\"submit\" value=\"Save\" onclick=\"return confirm(&quot;Are you sure you want to change the WiFi settings?&quot;);\"> </div></form><a href=\"/\">Back to top</a></div><div>";
		content += network_html;
		content += "</div></body></html>";
		_server.send(200, "text/html", content);
	});

	_server.on("/set_wifi_conf", [&]() {
		String new_ssid = _server.arg("ssid");
		String new_pwd = _server.arg("pwd");
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
			_server.send(200, "text/html", content);
			delay(1000);
			ESP.restart();
		}
		else {
			content += "<p>Empty SSID is not acceptable. </p>";
			content += "<a href=\"/\">Back to top</a></div><body></html>";
			Serial.println("Rejected empty SSID.");
			_server.send(200, "text/html", content);
		}
	});

	_server.on("/module_id", [&]() {
		char defaultId[sizeof(WiFiConf.module_id)];
		setDefaultModuleId(defaultId);
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Module ID</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		content += "</head><body><div><h1>Nefry Module ID</h1>";
		content += "<p>Module ID: ";
		content += WiFiConf.module_id;
		content += "</p>";
		content += "<form method='get' action='set_module_id'><div class=\"row\"> <label for=\"module_id\">New Module ID: </label> <div> <input name=\"module_id\" id=\"module_id\" maxlength=\"32\" value=\" ";
		content += WiFiConf.module_id;
		content += "\"></div></div>";
		content += "<div class=\"row\"> <label for=\"pwd\">Module wifi Pass: </label> <div> <input type=\"password\" name=\"pwd\" id=\"pwd\"maxlength=\"64\"> </div></div><div class = \"footer\"><input type='submit' value=\"Save\"onclick='return confirm(&quot;Are you sure you want to change the Module ID?&quot;);'></div></form>";
		content += " <p>Empty will reset to default ID '";
		content += defaultId;
		content += "'</p><a href=\"/\">Back to top</a></div></body></html>";
		_server.send(200, "text/html", content);
	});

	_server.on("/set_module_id", [&]() {
		String new_id = _server.arg("module_id");
		String new_pwd = _server.arg("pwd");
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Module Set</title>";
		content += css;
		content += "</head><body><div><h1>Nefry Module Set</h1>";
		if (new_id.length() > 0) {
			new_id.toCharArray(WiFiConf.module_id, sizeof(WiFiConf.module_id));
			new_pwd.toCharArray(WiFiConf.module_wifi_pwd, sizeof(WiFiConf.module_wifi_pwd));
			
		}
		else {
			resetModuleId();
		}
		saveConf();
		content += "<p>Set Module ID to '";
		content += WiFiConf.module_id;
		content += "' ... Restart. </p>";
		content += "<a href=\"/\">Back to top</a></div></body></html>";
		_server.send(200, "text/html", content);
		delay(2000);
		ESP.restart();
	});
	_server.on("/web_update", [&]() {
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\"><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		content += "<title>Nefry Web Update</title><style>.row>input,input[type=file]{width:100%}.row label{line-height:1.3;display:block;margin-bottom:5px;width:300px}.row>input{display:inline-block}</style>";
		content += "</head><body><div><h1>Nefry Web Update</h1>";
		content += "<form method='get' action='program'><div class=\"row\"> <label for=\"File\">Program download Domain: </label><input name='domain'id='URL' value=''><label for='File'>Program download URL(Domain except): </label><input name='URL'id='URL'  value=''placeholder=\"Null ok\">";
		content += "</div><div class=\"footer\"> <input type=\"submit\" value=\"Save\" onclick=\"\"></div></form></br><p>Default Program Download URL : wamisnet.github.io </p><a href='/'>Back to top</a></div>";
		content += "</body></html>";
		_server.send(200, "text/html", content);
	});
	_server.on("/program", [&]() {
		String program_domain = _server.arg("domain");
		String program_url = _server.arg("URL");
		_server.sendHeader("Connection", "close");
		_server.sendHeader("Access-Control-Allow-Origin", "*");
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Web Update</title>";// < link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		content += css;
		content += "</head><body><div><h1>Nefry Web Update</h1><p>";
		if (program_domain.length() > 0) {
			if (program_url.length() > 0)program_url = escapeParameter(program_url);
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
				Nefry_LED_blink(0x00, 0x4f, 0x00, 250, 10);
				Serial.println("[update] Update ok.");
				content += "[update] Update ok.";// may not called we reboot the ESP
				content += "</br></p><a href='/'>Back to top</a></div><body></html>";
				_server.send(200, "text/html", content);
				delay(2000);
				ESP.restart();
				break;
			}
		}
		else {
			content += "<p>Empty URL is not acceptable.</p>";
			Serial.println("Rejected empty URL.");
		}
		content += "</p><a href='/'>Back to top</a></div><body></html>";
		_server.send(200, "text/html", content);
	});
}
void Nefry_lib::setupWeb_local_Update(void) {
	int count = 0;
	_server.on("/update", HTTP_GET, [&]() {
		_server.sendHeader("Connection", "close");
		_server.sendHeader("Access-Control-Allow-Origin", "*");
		String sketchUploadForm = "<!DOCTYPE HTML><head><meta charset=\"UTF-8\">";
		sketchUploadForm += "<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		sketchUploadForm += "<title>Nefry Upload Sketch</title></head><body><div><h1>Upload Sketch</h1> <p>Upload a binary file of sketch.</p><form method=\"POST\" action=\"/upload_sketch\" enctype=\"multipart/form-data\">";
		sketchUploadForm += "<input type=\"file\" name=\"sketch\"><div class=\"footer\"> <input type=\"submit\" value=\"Upload\" onclick=\"return confirm(&quot;Are you sure you want to update the Sketch?&quot;);\">";
		sketchUploadForm += "</div></form><a href=\"/\">Back to top</a></div></body></html>";
		_server.send(200, "text/html", sketchUploadForm);
	});
	_server.onFileUpload([&]() {
		if (_server.uri() != "/upload_sketch") return;
		HTTPUpload& upload = _server.upload();
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
			_server.on("/upload_sketch", HTTP_POST, [&]() {
				_server.sendHeader("Connection", "close");
				_server.sendHeader("Access-Control-Allow-Origin", "*");
				String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
				content += "<title>Nefry Upload Sketch</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
				content += "</head><body><div><h1>Nefry Upload Err</h1>";
				content += "<p>File Err</p>";
				content += "<a href='/'>Back to top</a></div></body></html>";
				_server.send(200, "text/html", content);
			});
			return;
		}
	});
	_server.on("/upload_sketch", HTTP_POST, [&]() {
		_server.sendHeader("Connection", "close");
		_server.sendHeader("Access-Control-Allow-Origin", "*");
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Upload Sketch</title>";
		content += css;
		content += "</head><body><div><h1>Nefry Update</h1><p>";
		content += (Update.hasError()) ? "Update Err" : "Upload Success";
		content += "</p><a href='/'>Back to top</a></div></body></html>";
		_server.send(200, "text/html", content);
		delay(2000);
		ESP.restart();
	});
}
void Nefry_lib::setupWeb(void) {
	_server.on("/", [&]() {
		IPAddress ip = WiFi.localIP();
		String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry menu</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
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
		content += "<li><a href='/module_id'>Setup Module ID</a>";
		content += "<li><a href='/web_update'>Web Sketch Download</a>";
		content += "<li><a href='/update'>Upload Sketch</a>";
		content += "<li><a href='/console'>Web Console</a>";
		//if(User_page.length() > 0) {
		//	content += "<li><a href='/user'>Create Page</a>";
		//}
		content += "</ul>";
		content += "</div></body></html>";
		_server.send(200, "text/html", content);
	});
	_server.on("/nefry_css", [&]() {
		char content[] = "@charset \"UTF-8\"; *, :after, : before{ box - sizing:border - box }body{ font:16px / 1.65 \"Noto Sans\",\"HelveticaNeue\",\"Helvetica Neue\",Helvetica,Arial,sans-serif;margin:0;padding:0 20px;color:#555}ol,ul{padding-left:20px;margin:0}a{color:#54AFBA}a:hover{text-decoration:none}body>div{background:#fff;margin:20px auto;padding:20px 24px;box-shadow:0 0 1px 1px rgba(0,0,0,.1);border-radius:4px;max-width:540px}body>div input,body>div li{word-wrap:break-word}body>div>h1{font-size:1.4em;line-height:1.3;padding-bottom:4px;border-bottom:1px solid #efefef;margin-top:0;margin-bottom:20px}input,select,textarea{font:inherit inherit inherit}input{background:rgba(0,0,0,0);padding:.4em .6em;border:1px solid rgba(0,0,0,.12);border-radius:3px;-webkit-appearance:none;-moz-appearance:none;appearance:none}input:focus{border:1px solid #6E5F57;outline:0}input[type=submit]{margin-left:6px;cursor:pointer;line-height:2.6;display:inline-block;padding:0 1.2rem;text-align:center;vertical-align:middle;color:#FFF;border:0;border-radius:3px;background:#6E5F57;-webkit-appearance:none;-moz-appearance:none;appearance:none}.row,.row>div,.row>label{display:block}input[type=submit]:hover{color:#FFF;background:#372F2A}input[type=submit]:focus{outline:0}input[type=file]{width:100%}.row{margin-bottom:14px}.row>label{float:left;width:110px;font-size:14px;position:relative;top:8px}.row>div{margin-left:120px;margin-bottom:12px}.row>div>input{width:100%;display:inline-block}.footer{text-align:right;margin-top:16px}";
		_server.send(200, "text/css", content);
	});
}
void Nefry_lib::nefry_loop() {
	_server.handleClient();
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
int mojicount = 0, newwrite = 0;
void Nefry_lib::print(String text) {
	if (printcun > max_console) {
		Serial.println("over");
		printcun = 0;
	}
	Serial.println(text);
	text.toCharArray(moji, 60);
	strcpy(printweb[printcun], moji);
	printcun++;
	if (mojicount < max_console)
		mojicount++;
	newwrite = 1;
}
void Nefry_lib::println(String text) {
	print(text + "<br>");
}

void Nefry_lib::nefry_console() {
	_server.on("/console", [&]() {
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\"><meta http-equiv=\"Refresh\" content=\"5\">";
		content += "<title>Nefry Console</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		content += "</head><body><div><h1>Nefry Console</h1>";
		content += "<p>It can be used as a terminal.</p>";
		content += "<form method='get' action='set_console'><div class=\"row\"> <label for=\"console\">console:</label> <div> <input name=\"console\" id=\"console\" maxlength=\"100\" value=\"\"> </div></div>";
		content += "<div class=\"footer\"> <input type=\"submit\" value=\"Send\"> </div></form>";
		content += "<p><div>";
		int i;
		if (mojicount >= max_console)i = printcun + 1;
		else i = 0;
		for (int j = 0; j < mojicount; j++, i++) {
			if (i > max_console)i = 0;
			content += printweb[i];
		}
		content += "</div></p></br><a href=\"/\">Back to top</a></div>";
		content += "</body></html>";
		_server.send(200, "text/html", content);
	});

	_server.on("/set_console", [&]() {
		input_console = _server.arg("console");
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
		_server.send(200, "text/html", content);
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
	printWiFiConf();
}
bool Nefry_lib::memory_write(const char *pt,int num) {
	Serial.println("memory write");
	switch (num){
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
	default:
		return 0;
		break;
	}
	return 1;
}
char* Nefry_lib::memory_read(int num) {
	Serial.println("memory read");
	switch (num) {
	case 1:
		return WiFiConf.memo1;
		break;
	case 2:
		return WiFiConf.memo2;
		break;
	case 3:
		return WiFiConf.memo3;
		break;
	}
	return WiFiConf.memo1;
}

void Nefry_lib::Nefry_LED(char r, char g, char b, char w, char pin, int num) {
	_NefryLED[pin].setBrightness(w);
	_NefryLED[pin].setPixelColor(num, r, g, b);
	_NefryLED[pin].show();
}
void Nefry_lib::Nefry_LED_blink(char r, char g, char b, int wait, int loop, char pin) {
	int i = 0;
	while (i < loop) {
		Nefry_LED(r, g, b, 122, pin);
		delay(wait);
		Nefry_LED(r, g, b, 0, pin);
		delay(wait);
		i++;
	}
}
void Nefry_lib::Nefry_LED_begin(int num, int pin, uint8_t t = NEO_GRB + NEO_KHZ800) {
	_NefryLED[pin] = Adafruit_NeoPixel(num, pin, t);
	_NefryLED[pin].begin();
}

void Nefry_lib::reset() {
	ESP.restart();
}
void Nefry_lib::sleep(int sec) {
	ESP.deepSleep(sec * 1000 * 1000, WAKE_RF_DEFAULT);
	delay(1000);
}
/*
void Nefry_lib::webpage(const char url[20],String page,String link){
	char uri[30]="/user/";
	char *u=uri;
	strcpy(u+6,url);
	print(page);
	/*_server.on(url, [&]() {
		String pagee="<!DOCTYPE html><head><meta charset=\"UTF-8\"/><title>サイト名</title></head><body><h1>サイト名</h1></body></html>";
		_server.send(200, "text/html", pagee);
	});*//*
		_server.on(url, [&]() {
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
		_server.send(200, "text/html", content);
	});
	User_page +="<li><a href=\"";
	User_page +=url;
	User_page +="\">";
	User_page +=link;
	User_page +="</a>";
}
void Nefry_lib::user_webpage(){
	_server.on("/user", [&]() {
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
		_server.send(200, "text/html", content);
	});
	_server.on("/ard", [&]() {
		_server.send(200, "text/html","");
	});
}
*/
Nefry_lib Nefry;