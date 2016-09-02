/**
Nefry lib

Copyright (c) 2015 wami

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#define LIBVERSION ("2.0.1")
#include "Nefry.h"
const uint8_t wifi_conf_format[] = WIFI_CONF_FORMAT;
struct WiFiConfStruct {
	uint8_t format[4];//1
	char sta_ssid[32];//2
	char sta_pwd[64];//3
	char module_id[32];//4
	char module_class[32];//5
	char module_wifi_pwd[64];//6
	char Nefry_user[32];//7
	char Nefry_user_pass[32];//8
	int mode[8];//9
	char str128[3][128];
	char str64[5][64];
	int bootmode;
} WiFiConf = {
	WIFI_CONF_FORMAT,//1
	"Nefry",//2
	"Nefry-wifi",//3
	"",//4
	"",//5
	"",//6
	"",//7
	"",//8
	0,//9
};
Adafruit_NeoPixel _NefryLED[17];

//public

//etc
char* Nefry_lib::getModuleName() {
	return WiFiConf.module_id;
}
const char * program;
String Nefry_lib::getProgramName() {
	return program;
}
void Nefry_lib::setProgramName(const char *pn) {
	program = pn;
}

String Nefry_lib::getVersion() {
	return LIBVERSION;
}

bool Nefry_lib::push_SW() {
	pinMode(4, INPUT_PULLUP);
	if (digitalRead(4) == LOW)return true;
	return false;
}

void Nefry_lib::ndelay(unsigned long ms) {
	unsigned long beforetime = millis();
	while (millis() - beforetime < ms) {
		if (millis() < beforetime)break;
		nefry_loop();
		delay(1);
	}
}

void Nefry_lib::reset() {
	pinMode(16, OUTPUT);
	Serial.println("Nefry Reset");
	delay(10);
	digitalWrite(16, LOW);
	delay(100);
	ESP.restart();
	delay(500);
}

void Nefry_lib::sleep(const int sec) {
	ESP.deepSleep(sec * 1000 * 1000, WAKE_RF_DEFAULT);
	delay(1000);
}

//auth

bool Nefry_lib::login(const char *UserID, const char *User_pass) {
	if (strcmp(UserID, WiFiConf.Nefry_user) == 0)
		if (strcmp(User_pass, WiFiConf.Nefry_user_pass) == 0)
			return true;
	return false;
}

bool Nefry_lib::Auth(const char *Nefryclass, const char *NefryID) {
	if (strcmp(NefryID, WiFiConf.module_id) == 0)return true;
	if (strcmp(Nefryclass, WiFiConf.module_class) == 0)return true;
	return false;
}

//SetConf
bool htmlPrint[20];//10
void Nefry_lib::setConfHtmlPrint(const bool data, const int num) {
	if (0 <= num&&num < 20)
		htmlPrint[num] = data;
}
bool Nefry_lib::getConfHtmlPrint(const int num) {
	if (0 <= num&&num < 20)
		return htmlPrint[num];
}
void Nefry_lib::setConfHtml(const char set[15], const int num) {
	if (0 <= num&&num < 20){
		strcpy(module_input[num], set);
		setConfHtmlPrint(1, num);
	}

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

//ConfStr

bool Nefry_lib::setConfStr(const char *pt, int num) {
	Serial.println("memory write");
	if (num <= 2) {
		setConf(WiFiConf.str128[num], pt);
	}
	else if (num <= 7) {
		setConf(WiFiConf.str64[num - 3], pt);
	}
	else {
		return false;
	}
	return true;
}

char* Nefry_lib::getConfStr(const int num) {
	Serial.println("memory read");
	if (num <= 2) {
		return WiFiConf.str128[num];
	}
	else if (num <= 7) {
		return WiFiConf.str64[num - 3];
	}
	else {
		return (char*)"";
	}
}

//ConfValue

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

//web
String ipaddress;
void Nefry_lib::setupWebModuleConf(void) {
	nefry_server.on("/module_id", [&]() {
		char defaultId[sizeof(WiFiConf.module_id)];
		setDefaultModuleId(defaultId);
		String content = F("<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">"
					"<title>Nefry Module ID</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\"><link rel=\"stylesheet\" type=\"text/css\" href = \"/nefry_content\">"
					"</head><body><div><h1>Nefry Module Setup</h1>"
					"<div class=\"moduleid\">Module ID: </div>"
					"<form method='get' action='set_module_id'><div class=\"row\"> <label>NewModuleID: </label> <div> <input name=\"id\" maxlength=\"32\" value=\"");
		content += WiFiConf.module_id;
		content += F("\"></div></div>"
					"<div class = \"row\"><label>Module class: </label> <div> <input name=\"cls\" maxlength=\"32\" value=\"");
		content += WiFiConf.module_class;
		content += F("\"></div></div>";
					"<div class=\"row\"><label>Nefry WiFi Pass: </label> <div> <input type=\"password\" name=\"pwd\" maxlength=\"64\"> </div></div>"
					"<div class = \"row\"><label>Nefry User: </label> <div> <input name=\"user\" maxlength=\"32\" value=\"");
		content += WiFiConf.Nefry_user;
		content += F("\"></div></div>"
					"<div class = \"row\"><label>Nefry User Pass: </label> <div> <input type=\"password\" name=\"uPas\"maxlength=\"32\" value=\"\"></div></div>");
		for (int i = 0; i < 3; i++) {
			if (htmlPrint[i] == 1) {
				content += "<div class = \"row\"><label>";
				content += module_input[i];
				content += "</label> <div> <input name=\"mode";
				content += i;
				content += "\" maxlength=\"128\" value=\"";
				content += WiFiConf.str128[i];
				content += "\"></div></div>";
			}
		}
		for (int i = 3; i < 8; i++) {
			if (htmlPrint[i] == 1) {
				content += "<div class = \"row\"><label>";
				content += module_input[i];
				content += "</label> <div> <input name=\"mode";
				content += i;
				content += "\" maxlength=\"64\" value=\"";
				content += WiFiConf.str64[i - 3];
				content += "\"></div></div>";
			}
		}
		content += F("<div class=\"psrow\"><div><input type='submit' value=\"Save\"onclick='return confirm(&quot;Are you sure you want to change the Module ID?&quot;);'></div></form>"
					"<div><form method = 'get' action = 'reset'><input type='submit' value=\"Restart\"></form></div>"
					"<div><form method = 'get' action = 'onreset'><input type='submit' value=\"Write mode\"></form></div>"
					" </div><div>Empty will reset to default ID '");
		content += defaultId;
		content += "'</div><div>Nefry library:";
		content += getVersion();
		content += "</div><div>Running ProgramName:";
		content += getProgramName();
		content += F("</div><div class=\"writemode\"></div><a href=\"/module_id_next\">Next page</a></br><a href=\"/\">Back to top</a></div></body></html>");
		nefry_server.send(200, "text/html", content);
	});

	nefry_server.on("/set_module_id", [&]() {
		String new_id = nefry_server.arg("id");
		String new_class = nefry_server.arg("cls");
		String new_pwd = nefry_server.arg("pwd");
		String new_user = nefry_server.arg("user");
		String new_user_pass = nefry_server.arg("uPas");
		char webarg[10] = { "mode0" };
		for (int i = 0; i < 3; i++) {
			webarg[4] = '0' + i;
			print(webarg);
			String memo1 = nefry_server.arg(webarg);
			memo1.toCharArray(WiFiConf.str128[i], sizeof(WiFiConf.str128[i]));
		}
		for (int i = 3; i < 8; i++) {
			webarg[4] = '0' + i;
			print(webarg);
			String memo1 = nefry_server.arg(webarg);
			memo1.toCharArray(WiFiConf.str64[i - 3], sizeof(WiFiConf.str64[i - 3]));
		}

		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Module Set</title>";
		content += "<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
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

		saveConf();
		content += "<p>Set Module ID to '";
		content += WiFiConf.module_id;
		content += "' ... Restart. </p>";
		content += "<a href=\"/\">Back to top</a></div></body></html>";
		nefry_server.send(200, "text/html", content);
		ndelay(2000);
		reset();
	});
	nefry_server.on("/module_id_next", [&]() {
		int pCount = 0;
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Module ID</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\"><link rel=\"stylesheet\" type=\"text/css\" href = \"/nefry_content\">";
		content += "</head><body><div><h1>Nefry Module Setup</h1>";
		content += "<div class=\"moduleid\">Module ID: </div>";
		content += "<form method='get' action='set_module_id_next'>";
		for (int i = 0; i < 8; i++) {
			if (htmlPrint[i + 10] == 1) {
				content += "<div class = \"row\"> <label>";
				content += module_input[10 + i];
				content += "</label> <div> <input name=\"mode";
				content += i;
				content += "\" type=\"number\" value=\"";
				content += WiFiConf.mode[i];
				content += "\"></div></div>";
				pCount++;
			}
		}
		if (pCount > 0) {
			content += "<div class=\"psrow\"><div><input type = 'submit' value = \"Save\"onclick='return confirm(&quot;Are you sure you want to change the Module ID?&quot;);'></div>";
			content += "<div><form method ='get'actio ='reset'><input type='submit' value=\"Restart\"></form></div></div>";
		}
		content += "</form>";
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
		char webarg[10] = { "mode0" };
		for (int i = 0; i < 8; i++) {
			webarg[4] = '0' + i;
			print(webarg);
			String memo1 = nefry_server.arg(webarg);
			WiFiConf.mode[i] = memo1.toInt();
		}
		saveConf();
		String content = "<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">";
		content += "<title>Nefry Module Set</title>";
		content += "<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">";
		content += "</head><body><div><h1>Nefry Module Set</h1>";

		content += "<p>Set Module ID to '";
		content += WiFiConf.module_id;
		content += "' ... Restart. </p>";
		content += "<a href=\"/\">Back to top</a></div></body></html>";
		nefry_server.send(200, "text/html", content);
		ndelay(2000);
		reset();
	});

}

void Nefry_lib::setupWebWiFiConf(void) {
	nefry_server.on("/wifi_conf", [&]() {
		String content = F(
			"<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">"
			"<title>Nefry Wifi Set</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\"><style>.row>label{width:50px}</style>"
			"</head><body><div><h1>Nefry Wifi Set</h1>"
			"<form method='get' action='set_wifi'><div class=\"row\"> <label for=\"ssid\">SSID: </label> <div> <input name=\"ssid\" id=\"ssid\" maxlength=\"32\"list=\"network_list\" value=\"\"> </div></div>"
			"<div class=\"row\"> <label for=\"pwd\">PASS: </label> <div> <input type=\"password\" name=\"pwd\" id=\"pwd\"maxlength=\"64\"> </div></div>"
			"<div class=\"footer\"><button type = \"button\" onclick=\"location.href='/wifiReload'\">Reload</button><input type=\"submit\" value=\"Save\" onclick=\"return confirm(&quot;Are you sure you want to change the WiFi settings?&quot;);\"> </div></form><a href=\"/\">Back to top</a></div><div>"
		);
		content += network_html;
		content += network_list;
		content += "</div></body></html>";
		nefry_server.send(200, "text/html", content);
	});
	nefry_server.on("/wifiReload", [&]() {
		scanWiFi();
		IPAddress ip = WiFi.localIP();
		if (ip.toString().equals("0.0.0.0")) ipaddress = "192.168.4.1";
		else ipaddress = ip.toString();
		String content = F(
			"<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
			"<title>Wifi Reload</title></head><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">"
			"<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_content\">"
			"<meta http-equiv=\"Refresh\" content=\"0; URL = http://");
		content += ipaddress;
		content += "/wifi_conf\"><body><div>";
		content += "<h1 class=\"wifi\"></h1><p>Please wait...</p><a href=\"http://";
		content += ipaddress;
		content += "/wifi_conf\"><p class=\"wifi\"></p></a></div></body></html>";
		nefry_server.send(200, "text/html", content);
	});
	nefry_server.on("/set_wifi", [&]() {
		String new_ssid = escapeParameter(nefry_server.arg("ssid"));
		String new_pwd = escapeParameter(nefry_server.arg("pwd"));
		String content = F(
			"<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">"
			"<title>Nefry Wifi Set</title>"
			"<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_content\">"
			"<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">"
			"</head><body><div><h1>Nefry Wifi Set</h1>"
			"<p class=\"wifiSave\"></p>"
			"<a href=\"/\">Back to top</a></div><body></html>");
		if (new_ssid.length() > 0) {
			new_ssid.toCharArray(WiFiConf.sta_ssid, sizeof(WiFiConf.sta_ssid));
			new_pwd.toCharArray(WiFiConf.sta_pwd, sizeof(WiFiConf.sta_pwd));
			saveConf();
			String Ccontent = "Save SSID:";
			Ccontent += WiFiConf.sta_ssid;
			Ccontent += " Restart to boot into new WiFi";
			cssAdd("wifiSave", Ccontent);
			nefry_server.send(200, "text/html", content);
			ndelay(1000);
			reset();
		}
		else cssAdd("wifiSave", "Empty SSID is not acceptable.");
		nefry_server.send(200, "text/html", content);
	});
}

void Nefry_lib::setupWebLocalUpdate(void) {
	int count = 0;
	bool err = false;
	nefry_server.on("/update", HTTP_GET, [&]() {
		String content = F(
			"<!DOCTYPE HTML><head><meta charset=\"UTF-8\">"
			"<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">"
			"<title>Nefry Upload Sketch</title></head><body><div><h1>Upload Sketch</h1> <p>Upload a binary file of sketch.</p><form method=\"POST\" action=\"/upload_sketch\" enctype=\"multipart/form-data\">"
			"<input type=\"file\" name=\"sketch\"><div class=\"footer\"> <input type=\"submit\" value=\"Upload\" onclick=\"return confirm(&quotAre you sure you want to update the Sketch?&quot)\">"
			"</div></form><a href=\"/\">Back to top</a></div></body></html>");
		nefry_server.send(200, "text/html", content);
	});
	nefry_server.onFileUpload([&]() {
		if (nefry_server.uri() != "/upload_sketch") return;
		pushSW_flg = 1;
		HTTPUpload& upload = nefry_server.upload();
		Serial.printf("Sketch: %s Uploading\n", upload.filename.c_str());
		String file_name = String(upload.filename.c_str());
		count++;
		if (file_name.endsWith("bin")) {
			err = false;
			if (count % 20 == 1)
				Nefry_LED_blink(0x00, 0x00, 0xFF, 10, 1);
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
			pushSW_flg = 0;
			if (count % 15 == 1)
				Nefry_LED_blink(0xFF, 0x0, 0x00, 80, 1);
			Serial.println("err");
			err = true;
		}
	});
	nefry_server.on("/upload_sketch", HTTP_POST, [&]() {
		String content = F(
			"<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">"
			"<title>Nefry Upload Sketch</title>"
			"<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_content\">"
			"<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">"
			"</head><body><div><h1>Nefry Update</h1><div class=\"updateS\">"
			"</div><a href='/'>Back to top</a></div></body></html>");
		if (err) {
			cssAdd("updateS", "File Err. Failed to update");
			nefry_server.send(200, "text/html", content);
			pushSW_flg = 0;
		}
		else {
			cssAdd("updateS", (Update.hasError()) ? "Update Err" : "Upload Success");
			nefry_server.send(200, "text/html", content);
			ndelay(5000);
			pushSW_flg = 0;
			reset();
		}
	});
}

void Nefry_lib::setWebUpdate(String program_domain, String program_url) {
	pushSW_flg = 1;
	IPAddress ip = WiFi.localIP();
	String content = F(
		"<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">"
		"<title>Nefry Web Update</title>"
		"<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_content\">"
		"<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">"
		"</head><body><div><h1>Nefry Web Update</h1><div class=\"updateHS\">"
		"</div><a href='/'>Back to top</a></div><body></html>");
	if (ip.toString().equals("0.0.0.0")) {
		println("not connected to the Internet");
		cssAdd("updateHS", "It is not connected to the Internet.Please connect to the Internet .");
	}
	else {
		println("connected to the Internet");
		if (program_domain.length() > 0) {
			if (program_url.length() > 0)program_url = escapeParameter(program_url);
			print(program_url);
			program_url.concat("/arduino.bin");
			switch (ESPhttpUpdate.update(program_domain, 80, program_url)) {
			case HTTP_UPDATE_FAILED:
				Serial.println(program_url);
				Serial.println("[update] Update failed.");
				cssAdd("updateHS", "[update] Update failed.");
				break;
			case HTTP_UPDATE_NO_UPDATES:
				Serial.println("[update] Update no Update.");
				cssAdd("updateHS", "[update] Update no Update.");
				break;
			case HTTP_UPDATE_OK:
				cssAdd("updateHS", "[update] Update ok.");
				nefry_server.send(200, "text/html", content);
				ndelay(1000);
				Nefry_LED_blink(0x00, 0xff, 0xff, 250, 10);
				Serial.println("[update] Update ok.");
				ndelay(2000);
				setLed(0x00, 0xff, 0xff);
				reset();
				break;
			}
		}
		else {
			Serial.println("Rejected empty URL.");
			pushSW_flg = 0;
			cssAdd("updateHS", "Empty URL is not acceptable.");
		}
	}
	nefry_server.send(200, "text/html", content);
	pushSW_flg = 0;
}

void Nefry_lib::setupWebOnlineUpdate(void) {
	nefry_server.on("/web_update", [&]() {
		String content = F(
			"<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\"><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">"
			"<title>Nefry Web Update</title><style>.row>input,input[type=file]{width:100%}.row label{line-height:1.3;display:block;margin-bottom:5px;width:300px}.row>input{display:inline-block}</style>"
			"</head><body><div><h1>Nefry Web Update</h1>"
			"<form method='get' action='program'><div class=\"row\"> <label for=\"File\">Program download Domain: </label><input name='domain'id='URL' value=''><label for='File'>Program download URL(Domain except): </label><input name='URL'id='URL'  value=''placeholder=\"Null ok\">"
			"</div><div class=\"footer\"> <input type=\"submit\" value=\"Save\" onclick=\"\"></div></form><br><p>Default Program Download URL : wamisnet.github.io </p><a href='/'>Back to top</a></div>"
			"</body></html>");
		nefry_server.send(200, "text/html", content);
	});
	nefry_server.on("/program", [&]() {
		setWebUpdate(nefry_server.arg("domain"), nefry_server.arg("URL"));
	});
}

String cssConten;
void Nefry_lib::setupWebMain(void) {
	IPAddress ip = WiFi.localIP();
	if (ip.toString().equals("0.0.0.0")) ipaddress = "192.168.4.1";
	else ipaddress = ip.toString();
	nefry_server.on("/", [&]() {
		String content = F(
			"<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">"
			"<title>Nefry Menu</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">"
			"<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_content\">"
			"</head><body><div><h1>Hello from Nefry!</h1>"
			"<div class=\"cssid\">Wifi Sport: "
			"</div><div class=\"ipaddress\">IP Address: "
			"</div><div class=\"moduleid\">Module ID: "
			"</div><div class=\"writemode\">"
			"</div>"
			"<ul>"
			"<li><a href='/wifi_conf'>Setup WiFi</a>"
			"<li><a href='/module_id'>Setup Module</a>"
			"<li><a href='/web_update'>Web Sketch Download</a>"
			"<li><a href='/update'>Upload Sketch</a>"
			"<li><a href='/console'>Web Console</a>"
			"</ul>"
			"</div></body></html>");
		nefry_server.send(200, "text/html", content);
	});
	nefry_server.on("/nefry_css", [&]() {
		String content = F(
			"@charset \"UTF-8\"; *, :after, : before{ box - sizing:border - box }body{ font:16px / 1.65 \"Noto Sans\",\"HelveticaNeue\",\"Helvetica Neue\",Helvetica,Arial,sans-serif;margin:0;padding:0 20px;color:#555}"
			"ol,ul{padding-left:20px;margin:0}a{color:#54AFBA}a:hover{text-decoration:none}body>div{background:#fff;margin:20px auto;padding:20px 24px;box-shadow:0 0 1px 1px rgba(0,0,0,.1);border-radius:4px;max-width:540px}"
			"body>div input,body>div li{word-wrap:break-word}body>div>h1{font-size:1.4em;line-height:1.3;padding-bottom:4px;border-bottom:1px solid #efefef;margin-top:0;margin-bottom:20px}input,select,textarea{font:inherit inherit inherit}"
			"input{background:rgba(0,0,0,0);padding:.4em .6em;border:1px solid rgba(0,0,0,.12);border-radius:3px;-webkit-appearance:none;-moz-appearance:none;appearance:none}input:focus{border:1px solid #6E5F57;outline:0}"
			"input[type=submit],button[type=button]{margin-left:6px;cursor:pointer;line-height:2.6;display:inline-block;padding:0 1.2rem;text-align:center;vertical-align:middle;color:#FFF;border:0;border-radius:3px;background:#6E5F57;-webkit-appearance:none;-moz-appearance:none;appearance:none}"
			".row,.row>div,.row>label{display:block}input[type=submit]:hover{color:#FFF;background:#372F2A}input[type=submit]:focus{outline:0}input[type=file]{width:100%}.row{margin-bottom:14px}"
			".row>label{float:left;width:110px;font-size:14px;position:relative;top:8px}.row>div{margin-left:120px;margin-bottom:12px}.row>div>input{width:100%;display:inline-block}.footer{text-align:right;margin-top:16px}"
			".psrow{text-align: center;}.psrow>div{display:inline-block;margin:10px;}");
		nefry_server.send(200, "text/css", content);

	});
	nefry_server.on("/nefry_content", [&]() {
		nefry_server.send(200, "text/css", cssConten);
	});

	nefry_server.on("/reset", [&]() {
		String content = F(
			"<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">"
			"<title>Nefry Reset</title>"
			"<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">"
			"</head><body><div><h1>Nefry Reset</h1>"
			"<p>Reset start!</p>"
			"<a href=\"/\">Back to top</a></div></body></html>");
		nefry_server.send(200, "text/html", content);
		ndelay(2000);
		reset();
	});
	nefry_server.on("/onreset", [&]() {
		WiFiConf.bootmode = 1;
		delay(10);
		saveConf();
		String content = F(
			"<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">"
			"<title>Nefry Write mode</title>"
			"<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">"
			"</head><body><div><h1>Nefry Write mode</h1>"
			"<p>Reset start!</p>"
			"<a href=\"/\">Back to top</a></div></body></html>");
		nefry_server.send(200, "text/html", content);
		ndelay(2000);
		reset();
	});
}

void Nefry_lib::setupWebCaptivePortal(void) {
	_dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
	nefry_server.onNotFound([&]() {
		String content = F(
			"<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
			"<link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_content\">"
			"<title>CaptivePortal</title></head><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">  "
			"<meta http-equiv=\"Refresh\" content=\"0; URL = http://");
		content += ipaddress;
		content += "\"><body><div>";
		content += "<h1 >Move to main page!</h1><p>Please wait...</p><a href=\"http://";
		content += ipaddress;
		content += "\">Move to main page!</a></div></body></html>";
		nefry_server.send(200, "text/html", content);
	});
}

void Nefry_lib::setupWebCss(void) {
	cssAdd("ipaddress", ipaddress);
	cssAdd("cssid", WiFiConf.sta_ssid);
	cssAdd("moduleid", WiFiConf.module_id);
	cssAdd("wifi", "Move to Wi-Fi set!");
}

bool Nefry_lib::checkWebVersionFile() {
	/*File tmpf;
	tmpf = SPIFFS.open("version", "r");
	if (tmpf) {
		tmpf.close();
		return true;
	}
	else {
		return false;
	}*/
}

void Nefry_lib::downloadWebFile() {
	//1.3.0用
	/*nefry_server.onNotFound([&]() {
		String content = "<!DOCTYPE HTML><html><head>";
		content += "<title>Nefry</title></head><body><div>";
		content += "<h1 >There are no files necessary to Nefry.</h1><p>Please connect to the Internet.</p> ";
		content += "<form method='get' action='set'><div><label>SSID: </label><input name=\"ssid\" maxlength=\"32\"></div>";
		content += "<div><label>PASS: </label><input type=\"password\" name=\"pwd\"maxlength=\"64\"></div>";
		content += "<div><a href='/Re'>Reload</a><input type=\"submit\" value=\"Save\"></div></form></div></body></html>";
		nefry_server.send(200, "text/html", content);
		});
		nefry_server.on("/Re", [&]() {
		scanWiFi();
		nefry_server.send(200, "text/html", "Wifi reload OK!<a href=\"/\">Back to top</a>");
		});
		nefry_server.on("/set", [&]() {
		String new_ssid = escapeParameter(nefry_server.arg("ssid"));
		String new_pwd = escapeParameter(nefry_server.arg("pwd"));
		if (new_ssid.length() > 0) {
		new_ssid.toCharArray(WiFiConf.sta_ssid, sizeof(WiFiConf.sta_ssid));
		new_pwd.toCharArray(WiFiConf.sta_pwd, sizeof(WiFiConf.sta_pwd));
		saveConf();
		nefry_server.send(200, "text/plain", "seve OK!Reboot start!");
		ndelay(1000);
		reset();
		digitalWrite(16, LOW);
		}
		});
		IPAddress ip = WiFi.localIP();
		if (!ip.toString().equals("0.0.0.0")) {
		File tmpf;
		tmpf = SPIFFS.open("programH", "r");
		if (!tmpf) {
		Serial.println("connect update Program!");
		Serial.println("Html prog Download");
		delay(1000);
		tmpf = SPIFFS.open("programH", "w");
		if (tmpf) {
		tmpf.print(1);
		tmpf.close();
		Serial.println("file Write");
		}
		setWebUpdate("wamisnet.github.io", "/pg/html/1.3.0");
		}
		}
		*/
}

void Nefry_lib::setupWeb(void) {
	nefry_server.begin();
	//if (checkWebVersionFile()) {
	//Serial.println("version file ok");
	setupWebModuleConf();
	setupWebLocalUpdate();
	setupWebOnlineUpdate();
	setupWebWiFiConf();
	setupWebCaptivePortal();
	setupWebConsole();
	setupWebMain();
	setupWebCss();
	/*}
	else {
	Serial.println("version file null");
	downloadWebFile();
	}*/
	// start mDNS responder
	if (!MDNS.begin("nefry")) {
		Serial.println("Error setting up MDNS responder!");
		setLed(0xff, 0x0, 0x00);
		delay(1000);
	}
	Serial.println("\nmDNS responder started");
	// Add service to MDNS-SD
	MDNS.addService("http", "tcp", 80);
}

void Nefry_lib::cssAdd(const char* id, String data, bool afterflg) {
	String sCss = ".";
	sCss += id;
	if (afterflg)sCss += ":after";
	else sCss += ":before";
	sCss += "{content:\"";
	sCss += data;
	sCss += "\"}";
	cssConten += sCss;
}

void Nefry_lib::spiffsWeb(const char *fname, String stradd) {
	/*File tmpf;
	tmpf = SPIFFS.open(fname, "r");
	if (tmpf) {
		if (stradd.length() > 0)nefry_server.send(200, "text/html", tmpf.readString() + stradd);
		else nefry_server.send(200, "text/html", tmpf.readString());
		tmpf.close();
	}*/
}

//main 

void Nefry_lib::nefry_init() {	
	Serial.begin(115200);
	setupModule();
	Serial.println("\n\nNefry Startup");
	delay(10);
	push_sw_();
	setLed(0x00, 0x6f, 0x00);
	// scan Access Points
	printModule();
	Serial.println("WiFi Startup");
	setupWifi();
	printIpaddress();
	setLed(0x00, 0x8f, 0x00);
	push_sw_();
	// setup Web Interface
	Serial.println("\nServer started");
	setupWeb();
	push_sw_();
	
	setLed(0x00, 0xaf, 0x00);
	push_sw_();
	setLed(0x00, 0xff, 0xff);
	delay(1000);
	if (pushSW_flg==1) {
		WiFi.softAP(WiFiConf.module_id);
		for (int i = 0; i < 20;i++)
			setConfHtmlPrint(1, i);
		println("Nefry Write mode");
		setLed(0x0f, 0xff, 0xff);
		cssAdd("writemode", "Write Mode");
	}
}

void Nefry_lib::nefry_loop() {
	_dnsServer.processNextRequest();
	nefry_server.handleClient();
}

void Nefry_lib::setupWifi(void) {
	scanWiFi();
	push_sw_();
	// start WiFi
	WiFi.persistent(false);
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
}
bool Nefry_lib::autoConnect(int sec) {
	if (WiFi.status() != WL_CONNECTED) {
		int wait = 0;
		while (wait < 10 * sec) {
			if (WiFi.status() == WL_CONNECTED) {
				println("WiFi connected");
				return true;
			}
			Nefry_LED_blink(0x00, 0x4f, 0x00, 200, 1);
			print(".");
			wait++;
			ndelay(100);
		}
		WiFi.disconnect();
		println("Connect timed out");
		return false;
	}
	return true;
}

void Nefry_lib::setupModule(void) {
	if(boardId==1)
		beginLed(1, 0, NEO_RGB);
	else
		beginLed(1, 0, NEO_GRB);
	delay(1);
	Serial.println("\n\nStartup");
	setLed(0x00, 0x0f, 0x00);
	ESP.wdtDisable();
	ESP.wdtEnable(60000);
	pinMode(4, INPUT_PULLUP);
	setLed(0x00, 0x2f, 0x00);
	push_sw_();
	module_set();
	nefry_server = ESP8266WebServer(80);
	Serial.println("server On");
	Serial.println(sizeof(WiFiConf));
	EEPROM.begin(1034);
	setLed(0x00, 0x4f, 0x00);
	if (!loadConf()) {
		resetModule();
		saveConf();
	}
	Serial.println("memory Ok");
	pushSW_flg = WiFiConf.bootmode;//webオンライン書き込みモード変更
	if (pushSW_flg == 1) {
		WiFiConf.bootmode = 0;
		saveConf();
	}
}

//webConsole

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
char printweb[max_console][50];
int mojicount = 0;
void Nefry_lib::print(String text,int ln) {
	if (printcun >= max_console)printcun = 0;
	Serial.print(text);
	if (ln == 1)text += "<br>";
	text.toCharArray(printweb[printcun++], 50);
	if (mojicount < max_console)mojicount++;
}

void Nefry_lib::println(String text) {
	print(text,1);
	Serial.println();
}

int Nefry_lib::available() {
	return input_console.length();
}

String Nefry_lib::read() {
	String read_console = input_console;
	input_console = "";
	return read_console;
}
void Nefry_lib::setupWebConsole(void) {
	nefry_server.on("/console", [&]() {
		input_console = nefry_server.arg("console");
		if (input_console.length() > 0) {
			input_console = escapeParameter(input_console);
			println(input_console);
		}
		String content = F(
			"<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">"
			"<title>Nefry Console</title><link rel = \"stylesheet\" type = \"text/css\" href = \"/nefry_css\">"
			"<script type = \"text/javascript\">\n"
			"  function loadDoc() {\n"
			"  if (window.XMLHttpRequest) {\n"
			"    xmlhttp = new XMLHttpRequest();\n"
			"  } else {\n"
			"    xmlhttp = new ActiveXObject(\"Microsoft.XMLHTTP\");\n"
			"  }\n"
			"  xmlhttp.onreadystatechange = function() {\n"
			"    if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {\n"
			"      document.getElementById(\"ajaxDiv\").innerHTML=xmlhttp.responseText;\n"
			"      console.log(\"get\");\n"
			"    }\n"
			"  }\n"
			"  xmlhttp.open(\"GET\",\"cons\",true);\n"
			"  xmlhttp.send();\n"
			"}\n"
			"var timer; \n"
			"timer = setInterval(\"loadDoc()\",2000);\n"
			"function reload(time) {\n"
			"  clearInterval(timer); \n"
			"  timer = setInterval(\"loadDoc()\",time);\n"
			"}\n"
			"</script>\n"
			"</head><body><div><h1>Nefry Console</h1>"
			"<p>It can be used as a terminal.</p>"
			"<form method='post' action='console'><div class=\"row\"> <label for=\"console\">console:</label> <div> <input name=\"console\" id=\"console\" maxlength=\"100\" value=\"\"> </div></div>"
			"<div class=\"footer\"> <input type=\"submit\" value=\"Send\"> </div></form>"
			"<p><div><div id=\"ajaxDiv\"></div><button type=\"button\" onclick=\"loadDoc()\">reload</button>"
			"<button type = \"button\" onclick=\"reload(500);\">0.5sec reload</button>"
			"<button type = \"button\" onclick=\"reload(2000);\">2sec reload</button>"
			"<button type = \"button\" onclick=\"reload(5000);\">5sec reload</button>"
			"<button type = \"button\" onclick=\"clearInterval(timer);\">stop</button>"
			"</div></p><br><a href=\"/\">Back to top</a></div>"
			"</body></html>");
		nefry_server.send(200, "text/html", content);
	});
	nefry_server.on("/cons", HTTP_GET, [&]() {
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
}

//LED

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

void Nefry_lib::setLed(const char r, const char g, const char b, const char w, const char pin, const int num) {
	_NefryLED[pin].setBrightness(w);
	_NefryLED[pin].setPixelColor(num, r, g, b);
	_NefryLED[pin].show();
}

//private
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
		htmlPrint[i] = 0;
	}
	for (int i = 10; i < 18; i++) {
		sprintf(module_input[i], "mode %d", i - 10);
		htmlPrint[i] = 0;
	}
}

void Nefry_lib::setDefaultModuleId(char* dst) {
	uint8_t macAddr[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(macAddr);
	if (boardId==2)
		sprintf(dst, "CocoaBit-%02x%02x", macAddr[WL_MAC_ADDR_LENGTH - 2], macAddr[WL_MAC_ADDR_LENGTH - 1]);
	else
		sprintf(dst, "Nefry-%02x%02x", macAddr[WL_MAC_ADDR_LENGTH - 2], macAddr[WL_MAC_ADDR_LENGTH - 1]);
}

void Nefry_lib::resetModule(void) {
	uint8_t macAddr[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(macAddr);
	setDefaultModuleId(WiFiConf.module_id);
	Serial.print("Reset Module ID to default: ");
	Serial.println(WiFiConf.module_id);
}

void Nefry_lib::printModule(void) {
	Serial.println();
	Serial.print("Name: ");
	Serial.println(WiFiConf.module_id);
	Serial.print("SSID: ");
	Serial.println(WiFiConf.sta_ssid);
	Serial.print("Module ID: ");
	Serial.println(WiFiConf.module_id);
}
void Nefry_lib::printIpaddress() {
	Serial.print("LAN: ");
	Serial.println(WiFi.localIP());
	Serial.print("AP: ");
	Serial.println(WiFi.softAPIP());
}

void Nefry_lib::saveConf(void) {
	Serial.println("writing WiFiConf");
	for (int i = 0; i < 4; i++) {
		WiFiConf.format[i] = wifi_conf_format[i];
		Serial.println(WiFiConf.format[i]);
	}
	EEPROM.write(0,0);
	for (unsigned int t = 0; t < sizeof(WiFiConf); t++) {
		EEPROM.write(WIFI_CONF_START + t, *((char*)&WiFiConf + t));
	}
	EEPROM.commit();
	delay(1);
}

bool Nefry_lib::loadConf() {
	Serial.println();
	Serial.println("loading WiFiConf");
	for (unsigned int t = 0; t < sizeof(WiFiConf); t++) {
		*((char*)&WiFiConf + t) = EEPROM.read(WIFI_CONF_START + t);
		Serial.print(EEPROM.read(WIFI_CONF_START + t));
	}
	Serial.println();
	for (int i = 0; i < 4; i++) {
		Serial.print("load format : ");
		Serial.print(wifi_conf_format[i]);
		Serial.print(" : EEPROM format : ");
		Serial.println(EEPROM.read(WIFI_CONF_START + i));
	}
	if (EEPROM.read(WIFI_CONF_START + 0) == wifi_conf_format[0] &&
		EEPROM.read(WIFI_CONF_START + 1) == wifi_conf_format[1] &&
		EEPROM.read(WIFI_CONF_START + 2) == wifi_conf_format[2] &&
		EEPROM.read(WIFI_CONF_START + 3) == wifi_conf_format[3])
	{
		for (unsigned int t = 0; t < sizeof(WiFiConf); t++) {
			*((char*)&WiFiConf + t) = EEPROM.read(WIFI_CONF_START + t);
		}
		return true;
	}
	else {
		Serial.println("WiFiConf was not saved on EEPROM.");
		return false;
	}
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
		network_html = "<ol>";
		network_list = "<datalist id=\"network_list\">";
		for (int i = 0; i < founds; ++i) {
			// Print SSID and RSSI for each network found
			Serial.print(i + 1);
			Serial.print(": ");
			Serial.print(WiFi.SSID(i));
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));
			Serial.println(")");
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
}
int Nefry_lib::waitConnected(void) {
	int wait = 0;
	Serial.println();
	Serial.println("Waiting for WiFi to connect");
	while (wait < 28) {
		if (WiFi.status() == WL_CONNECTED) {
			Serial.println("WiFi connected");
			return (1);
			ESP.wdtFeed();
		}
		Nefry_LED_blink(0x00, 0x4f, 0x00, 200, 1);
		Serial.print(".");
		push_sw_();
		wait++;
		delay(250);
	}
	WiFi.disconnect();
	Serial.println("");
	Serial.println("Connect timed out");
	return (0);
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
}*/

Nefry_lib Nefry;