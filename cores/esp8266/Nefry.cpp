/**
Nefry lib

Copyright (c) 2015 wami

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/
#define LIBVERSION ("2.2.4")
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
	char save_ssid[5][32];
	char save_pwd[5][64];
	int ptssid[5];
} WiFiConf = {
	WIFI_CONF_FORMAT,//1
	"Nefry",//2
	"Nefry-wifi",//3
	"",//4
	"",//5
	"",//6
	"",//7
	"",//8
	{0,0,0,0,0,0,0,0},//9
	{"","",""},
	{"","","","",""},
	0,
	{ "Nefry","","","",""},
	{ "Nefry-Wifi","","","",""},
	{1,0,0,0,0}
};
Adafruit_NeoPixel _NefryLED[17];
ESP8266WiFiMulti wifiMulti;
//public

//etc
char* Nefry_lib::getModuleName() {
	return WiFiConf.module_id;
}
ESP8266WebServer* Nefry_lib::getWebServer(void)
{
	return &nefry_server;
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

String Nefry_lib::ipaddressStr(IPAddress ip)
{
	if (ip.toString().equals("0.0.0.0"))return "192.168.4.1";
	else return ip.toString();
}

bool Nefry_lib::push_SW() {
	pinMode(4, INPUT_PULLUP);
	if (digitalRead(4) == LOW)return true;
	return false;
}

bool Nefry_lib::readSW()
{
	return push_SW();
}

void Nefry_lib::ndelay(unsigned long ms) {
	unsigned long beforetime = millis();
	while (millis() - beforetime < ms) {
		if (millis() < beforetime)break;
		nefry_loop();
		delay(1);
		yield();
	}
}

void Nefry_lib::reset() {
	pinMode(16, OUTPUT);
	Serial.println(F("Nefry Reset"));
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

/* HTMLに表示するのか */

void Nefry_lib::setConfHtmlPrint(const bool data, const int num) {
	if (0 <= num&&num < 20)
		htmlPrint[num] = data;
}
bool Nefry_lib::getConfHtmlPrint(const int num) {
	if (0 <= num&&num < 20)
		return htmlPrint[num];
	return false;
}
void Nefry_lib::setConfHtml(const char set[15], const int num) {
	if (0 <= num&&num < 20) {
		strcpy(module_input[num], set);
		setConfHtmlPrint(1, num);
	}
}

void Nefry_lib::setConfHtmlStr(const char set[15], const int num) {
	if (0 <= num&&num < 8) {
		strcpy(module_input[num], set);
		setConfHtmlPrint(1, num);
	}

}
void Nefry_lib::setConfHtmlValue(const char set[15], const int num) {
	if (0 <= num&&num < 8) {
		strcpy(module_input[num + 10], set);
		setConfHtmlPrint(1, num + 10);
	}

}

void Nefry_lib::setConf(char *old, const char *newdata) {
	if (strcmp(old, newdata) != 0) {
		strcpy(old, newdata);
		saveConf();
	}
}

void Nefry_lib::setConfWifi(const char SSID[32], const char pass[64]) {
	addWifi(SSID, pass);
	/*setConf(WiFiConf.sta_ssid, SSID);
	setConf(WiFiConf.sta_pwd, pass);*/
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
	Serial.println(F("memory write"));
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
	Serial.println(F("memory read"));
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
	Serial.println(F("memory read"));
	if (0 <= num && num <= 7)
		return WiFiConf.mode[num];
	else
		return 0;
}

//web

void Nefry_lib::setIndexLink(const char title[32], const char url[32])
{
	indexlink += "<li><a href='";
	indexlink += url;
	indexlink += "'>";
	indexlink += title;
	indexlink += "</a>";
}

String Nefry_lib::serectForm()
{
	String content = "";
	int formNumber;
	for (formNumber = 0; formNumber < 3; formNumber++) {
		if (htmlPrint[formNumber] == 1) {
			content += F("<div class=\"row\"><label>");
			content += module_input[formNumber];
			content += F("</label><div><input name=\"smo");
			content += formNumber;
			content += F("\"maxlength=\"128\"value=\"");
			content += WiFiConf.str128[formNumber];
			content += F("\"></div></div>");
		}
	}
	for (formNumber = 3; formNumber < 8; formNumber++) {
		if (htmlPrint[formNumber] == 1) {
			content += F("<div class=\"row\"><label>");
			content += module_input[formNumber];
			content += F("</label><div><input name=\"smo");
			content += formNumber;
			content += F("\"maxlength=\"64\"value=\"");
			content += WiFiConf.str64[formNumber - 3];
			content += F("\"></div></div>");
		}
	}
	for (formNumber = 10; formNumber < 18; formNumber++) {
		if (htmlPrint[formNumber] == 1) {
			content += F("<div class=\"row\"><label>");
			content += module_input[formNumber];
			content += F("</label><div><input name=\"imo");
			content += formNumber - 10;
			content += F("\"type=\"number\"value=\"");
			content += WiFiConf.mode[formNumber - 10];
			content += F("\"></div></div>");
		}
	}
	return content;
}

String Nefry_lib::createHtml(String title, String head, String body)
{
	String content = F(
		"<!DOCTYPE HTML><html><head><meta charset=\"UTF-8\">"
		"<script type=\"text/javascript\"src=\"jsform\"></script>"
		"<link rel=\"stylesheet\"type=\"text/css\"href=\"/nefry_content\">"
		"<link rel=\"stylesheet\"type=\"text/css\"href=\"/nefry_css\">"
		"<title>");
	content += title;
	content += F("</title>");
	content += head;
	content += F("</head><body><div>");
	content += body;
	content += F("</div><body></html>");
	/*Serial.println(title);
	Serial.println(head);
	Serial.println(body);*/
	return content;
}

//String ipaddress;
void Nefry_lib::setupWebModuleConf(void) {
	nefry_server.on("/module_id", [&]() {
		char defaultId[sizeof(WiFiConf.module_id)];
		setDefaultModuleId(defaultId);
		String content = F("<h1>Nefry Module Setup</h1><form method='get'action='set_module_id'><div class=\"row\"><label>Module ID:</label><div><input name=\"id\"maxlength=\"32\"value=\"");
		content += WiFiConf.module_id;
		content += F("\"></div></div>"
			"<div class=\"row\"><label>Module class:</label><div><input name=\"cls\"maxlength=\"32\"value=\"");
		content += WiFiConf.module_class;
		content += F("\"></div></div>"
			"<div class=\"row\"><label>Nefry WiFi Pass:</label><div><input type=\"password\"name=\"pwd\"maxlength=\"64\"></div></div>"
			"<div class=\"row\"><label>Nefry User:</label><div><input name=\"user\"maxlength=\"32\"value=\"");
		content += WiFiConf.Nefry_user;
		content += F("\"></div></div><div class = \"row\"><label>Nefry User Pass:</label><div><input type=\"password\"name=\"uPas\"maxlength=\"32\"value=\"\"></div></div>");
		content += serectForm();
		delay(1);
		content += F("<div class=\"psrow\"><div><input type=\"button\"value=\"Save\"onclick=\"return jsSubmit(this.form);\"></div></form>"
			"<div><form method='get'action='reset'><input type=\"button\"value=\"Restart\"onclick=\"return jsSubmit(this.form);\"></form></div>"
			"<div><form method='get'action='onreset'><input type=\"button\"value=\"Write Mode\"onclick=\"return jsSubmit(this.form);\"></form></div>"
			" </div><div>Empty will reset to default ID '");
		content += defaultId;
		content += F("</div><p>macAddress : ");
		content += WiFi.macAddress();
		content += F("</br>IPAddress : ");
		content += ipaddressStr(WiFi.localIP());
		content += F("</br>subNetMask : ");
		content += ipaddressStr(WiFi.subnetMask());
		content += F("</br>Gateway IPAddress : ");
		content += ipaddressStr(WiFi.gatewayIP());
		content += F("</p><div class=\"writemode\"></div><a href=\"/\">Back to top</a>");
		nefry_server.send(200, "text/html", createHtml(F("Nefry Module Config"),"",content));
	});

	nefry_server.on("/set_module_id", [&]() {
		String new_id = nefry_server.arg("id");
		String new_class = nefry_server.arg("cls");
		String new_pwd = nefry_server.arg("pwd");
		String new_user = nefry_server.arg("user");
		String new_user_pass = nefry_server.arg("uPas");
		char webarg[5] = { "smo0" };
		for (int i = 0; i < 3; i++) {
			webarg[3] = '0' + i;
			print(webarg);
			String memo1 = nefry_server.arg(webarg);
			memo1.toCharArray(WiFiConf.str128[i], sizeof(WiFiConf.str128[i]));
		}
		ndelay(10);
		for (int i = 3; i < 8; i++) {
			webarg[3] = '0' + i;
			print(webarg);
			String memo1 = nefry_server.arg(webarg);
			memo1.toCharArray(WiFiConf.str64[i - 3], sizeof(WiFiConf.str64[i - 3]));
		}
		webarg[0] = 'i';
		for (int i = 0; i < 8; i++) {
			webarg[3] = '0' + i;
			print(webarg);
			String memo1 = nefry_server.arg(webarg);
			WiFiConf.mode[i] = memo1.toInt();
		}
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
		nefry_server.send(200, "text/html", createHtml(F("Nefry Module Set"),"",
			(String)F("<h1>Nefry Module Set</h1><p>Set Module ID to '")+ WiFiConf.module_id +(String)F("'... Restart.</p><a href=\"/\">Back to top</a>")));
		ndelay(2000);
		reset();
	});
}

void Nefry_lib::setupWebWiFiConf(void) {
	nefry_server.on("/wifi_conf", [&]() {
		ndelay(1);
		String content = F(
			"<h1>Nefry Wifi Set</h1>"
			"<form  name=\"myForm\" method='get' action='set_wifi'><div class=\"row\"> <label for=\"ssid\">SSID: </label> <div> <input name=\"ssid\" id=\"ssid\" maxlength=\"32\"list=\"network_list\" value=\"\"> </div></div>"
			"<div class=\"row\"> <label for=\"pwd\">PASS: </label> <div> <input type=\"password\" name=\"pwd\" id=\"pwd\"maxlength=\"64\"> </div></div>"
			"<div class=\"footer\"><button type = \"button\" onclick=\"location.href='/wifiReload'\">Reload</button><input type=\"button\" value=\"Sava\" onclick=\"return jsSubmit(this.form);\"></div></form><a href=\"/\">Back to top</a></div><div>"
			);
		content += network_html;
		content += network_list;
		content += F("</div><div><h1>Saved WiFi List</h1><p>Delete WiFi Select</p><form  name=\"myForm\" method='get' action='delete_wifi'>");
		for (int i = 0; i < 5; i++) {
			if (WiFiConf.ptssid[i] != 0) {
				content += F("<input type=\"checkbox\" value=\"1\"name=\"");
				content += i;
				content += F("\">");
				content += WiFiConf.save_ssid[i];
				content += F("<br>");
			}
		}
		content += F("<input type=\"button\" value=\"Delete\" onclick=\"return jsSubmit(this.form);\"></form>");
		nefry_server.send(200, "text/html", createHtml(F("Nefry Wifi Confing"),"",content));
	});
	nefry_server.on("/wifiReload", [&]() {
		scanWiFiHtml();
		nefry_server.send(200, "text/html", createHtml(F("Wifi Reload"),
			(String)F("<meta http-equiv=\"Refresh\" content=\"0; URL = http://")+ ipaddressStr(WiFi.localIP()) +(String)F("/wifi_conf\">"),F("<p>Please wait...</p><a href=\"/wifi_conf\">")));
	});
	nefry_server.on("/set_wifi", [&]() {
		String new_ssid = escapeParameter(nefry_server.arg("ssid"));
		String new_pwd = escapeParameter(nefry_server.arg("pwd"));
		String content;
		if (new_ssid.length() > 0) {
			addWifi(new_ssid, new_pwd);
			content = F("Save SSID:");
			content += new_ssid;
			content += F(" Restart to boot into new WiFi");
		}
		else content=F("Empty SSID is not acceptable.");
		nefry_server.send(200, "text/html", createHtml(F("Nefry Wifi Set"), "",
			(String)F("<h1>Nefry Wifi Set</h1><p>")+content+(String)F("</p><a href=\"/\">Back to top</a>")));
		if (new_ssid.length() > 0) {
			ndelay(1000);
			reset();
		}
	});
	nefry_server.on("/delete_wifi", [&]() {
		String del="";
		for (int i = 0; i < 5; i++) {
			String data = nefry_server.arg(i);
			if (data.equals("1")) {
				del += "<li>";
				del += WiFiConf.save_ssid[i];
				del += "</li>";
				deleteWifi(i);
			}
		}
		sortWifi();
		saveWifi();
		nefry_server.send(200, "text/html", createHtml(F("Nefry Wifi Delete"),"",
			(String)F("<h1>Nefry Wifi Delete</h1><p>Delete List</p><ul>")+ del +(String)F("</ul><a href=\"/\">Back to top</a>")));
		ndelay(1000);
		reset();
	});
}

void Nefry_lib::setupWebLocalUpdate(void) {
	int count = 0;
	bool err = false;
	nefry_server.on("/update", HTTP_GET, [&]() {
		print("UPDNF");
		nefry_server.send(200, "text/html", createHtml(F("Nefry Upload Sketch"),"",
			F("<h1>Upload Sketch</h1> <p>Upload a binary file of sketch.</p><form method=\"POST\" action=\"/upload_sketch\" enctype=\"multipart/form-data\">"
			"<input type=\"file\" name=\"sketch\"><div class=\"footer\"><input type=\"submit\"value=\"Upload\"></div></form><a href=\"/\">Back to top</a>")));
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
			if (count % 5 == 1) {
				setLed(0xff, 0xff, 0x0);
				delay(10);
				setLed(0xff, 0xff, 0x0, 0);
			}
			if (upload.status == UPLOAD_FILE_START) {
				Serial.println("ok");
				Serial.setDebugOutput(true);
				WiFiUDP::stopAll();
				uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
				if (!Update.begin(maxSketchSpace))Update.printError(Serial);
			}
			else if (upload.status == UPLOAD_FILE_WRITE) {
				if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
					Update.printError(Serial);
					setLed(0xFF, 0x0, 0x0);
					delay(1000);
					setLed(0xFF, 0x0, 0x0, 0);
					err = true;
					println(F("File Err. Failed to update"));
					pushSW_flg = 0;
				}
			}
			else if (upload.status == UPLOAD_FILE_END) {
				if (Update.end(true)) {
					Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
					pushSW_flg = 0;
					setLed(0x0, 0x0, 0xFF);
					delay(1500);
					setLed(0xFF, 0x0, 0x0, 0);

				}
				else {
					Update.printError(Serial);
					setLed(0xFF, 0x0, 0x0);
					delay(1000);
					setLed(0xFF, 0x0, 0x0, 0);
					err = true;
					println(F("File Err. Failed to update"));
					pushSW_flg = 0;
				}
				Serial.setDebugOutput(false);
			}
			yield();
		}
		else {
			pushSW_flg = 0;
			if (count % 15 == 1) {
				setLed(0xFF, 0x0, 0x0);
				delay(10);
				setLed(0xFF, 0x0, 0x0, 0);
			}
			Serial.println(F("err"));
			println(F("File Err. Failed to update"));
			err = true;
			pushSW_flg = 0;
		}
	});

	nefry_server.on("/upload_sketch", HTTP_POST, [&]() {
		ClearConsole();
		if (err == false) {
			println((Update.hasError()) ? "Update Err" : "Upload Success");
		}
		else {
			println(F("Failed to update"));
		}
		nefry_server.send(200, "text/html", createHtml(F("Nefry Upload Sketch"),
			F("<script type=\"text/javascript\" src=\"consolejs\"></script><script type=\"text/javascript\">clearInterval(timer);loadDoc();</script>"),
			F("<h1>Nefry Update</h1><div id=\"ajaxDiv\"></div><a href='/'>Back to top</a>")));
		ndelay(5000);
		if (err == false) {
			reset();
		}
	});
}
int Nefry_lib::autoUpdate( String url, String domain) {
	pushSW_flg = 1;
	IPAddress ip = WiFi.localIP();
	if (ip.toString().equals("0.0.0.0")) {
		println(F("not connected to the Internet"));
	}
	else {
		println(F("autoUpdateStart"));
		ESPhttpUpdate.rebootOnUpdate(false);
		switch (ESPhttpUpdate.update(domain, 80, "/nefry.php", url)) {
		case HTTP_UPDATE_FAILED:
			pushSW_flg = 0;
			Serial.println(url);
			println(F("[update] Update failed."));
			Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
			println(ESPhttpUpdate.getLastErrorString().c_str());
			return ESPhttpUpdate.getLastError();
			break;
		case HTTP_UPDATE_NO_UPDATES:
			pushSW_flg = 0;
			println(F("[update] no Update."));
			return true;
			break;
		case HTTP_UPDATE_OK:
			println(F("[update] Update ok."));
			delay(2000);
			reset();
			break;
		}
	}
	pushSW_flg = 0;
	return false;
}

void Nefry_lib::setWebUpdate(String program_domain, String program_url) {
	pushSW_flg = 1;
	ClearConsole();
	IPAddress ip = WiFi.localIP();
	nefry_server.send(200, "text/html", createHtml(F("Nefry Web Update"),
		F("<script type=\"text/javascript\" src=\"consolejs\"></script><script type=\"text/javascript\">reload(10000);</script>"),
		F("<h1>Nefry Web Update</h1><p>自動で読み込まれるのでしばらくお待ちください。</p><div id=\"ajaxDiv\"></div><a href='/'>Back to top</a>")));
	ndelay(500);
	if (ip.toString().equals("0.0.0.0")) {
		println(F("Internet connection ... NG"));
		println(F("[UPDATE]It is not connected to the Internet.Please connect to the Internet ."));
	}
	else {
		println(F("Internet connection ... OK"));
		if (program_domain.length() > 0) {
			if (program_url.length() > 0)program_url = escapeParameter(program_url);
			//print(program_url);
			program_url.concat("/arduino.bin");
			ESPhttpUpdate.rebootOnUpdate(false);
			String UPurl;
			switch (ESPhttpUpdate.update(program_domain, 80, program_url)) {
			case HTTP_UPDATE_FAILED:
				Serial.println(program_url);
				UPurl += program_domain;
				UPurl += program_url;
				print(F("[UPDATE]アップデートに必要な情報が"));
				println(F("正しくありません。Update failed."));
				print(F("[UPDATE]URLを確認してください。"));
				print(F("Please check this URL : http://"));
				println(UPurl);
				print("ENDUP");
				break;
			case HTTP_UPDATE_NO_UPDATES:
				print(F("[UPDATE] アップデートはありません。"));
				println(F("	Update no Updates."));
				print("ENDUP");
				break;
			case HTTP_UPDATE_OK:
				print(F("[UPDATE] 更新完了、再起動します。"));
				println(F("Update OK"));
				print("ENDUP");
				ndelay(1000);
				Nefry_LED_blink(0x00, 0xff, 0xff, 250, 10);
				Serial.println(F("[update] Update ok."));
				ndelay(4000);
				setLed(0x00, 0xff, 0xff);
				ndelay(1000);
				reset();
				break;
			}
		}
		else {
			Serial.println(F("Rejected empty URL."));
			pushSW_flg = 0;
			println(F("[UPDATE]Empty URL is not acceptable."));
			print("ENDUP");
		}
	}

	pushSW_flg = 0;
}

void Nefry_lib::setupWebOnlineUpdate(void) {
	nefry_server.on("/web_update", [&]() {
		nefry_server.send(200, "text/html", createHtml(F("Nefry Web Update"), F("<style>label{margin:6px;width:200px;}input{margin:6px;width:95%;}</style>"),
			F("<h1>Nefry Web Update</h1>"
			"<form method='get' action='program'><label>Program download Domain</label><input name='domain'id='URL' value='program.nefry.studio'><label>Program download URL</label><input name='URL'id='URL'  value=''>"
			"<input type=\"button\" value=\"update\" onclick=\"return jsSubmit(this.form);\"></form><br><p>Default Program Download URL : program.nefry.studio</p><a href='/'>Back to top</a>")));
	});
	nefry_server.on("/program", [&]() {
		setWebUpdate(nefry_server.arg("domain"), nefry_server.arg("URL"));
	});
}

String cssConten;
void Nefry_lib::setupWebMain(void) {
	nefry_server.on("/", [&]() {
		String content = F("<h1>Hello from Nefry!</h1>"
			"<div>Wifi Sport: ");
		content += WiFi.SSID();
		content += F("</div><div>IP Address: ");
		content += ipaddressStr(WiFi.localIP());
		content += F("</div><div class=\"moduleid\">Module ID: "
			"</div><div class=\"writemode\">"
			"</div><ul>"
			"<li><a href='/wifi_conf'>Setup WiFi</a>"
			"<li><a href='/module_id'>Setup Module</a>"
			"<li><a href='/web_update'>Web Sketch Download</a>"
			"<li><a href='/update'>Upload Sketch</a>"
			"<li><a href='/console'>Web Console</a>");
		content += indexlink;
		content += F("</ul><p>Nefry library:");
		content += getVersion();
		content += F("</br>Running ProgramName:");
		content += getProgramName();
		content += F("</p>");
		nefry_server.send(200, "text/html", createHtml(F("Nefry Menu"),"",content));
	});
	nefry_server.on("/nefry_css", [&]() {
		nefry_server.send(200, "text/css", 
			F("@charset \"UTF-8\"; *, :after, : before{ box - sizing:border - box }body{ font:16px / 1.65 \"Noto Sans\",\"HelveticaNeue\",\"Helvetica Neue\",Helvetica,Arial,sans-serif;margin:0;padding:0 20px;color:#555}"
			"ol,ul{padding-left:20px;margin:0}a{color:#54AFBA}a:hover{text-decoration:none}body>div{background:#fff;margin:20px auto;padding:20px 24px;box-shadow:0 0 1px 1px rgba(0,0,0,.1);border-radius:4px;max-width:540px}"
			"body>div input,body>div li{word-wrap:break-word}body>div>h1{font-size:1.4em;line-height:1.3;padding-bottom:4px;border-bottom:1px solid #efefef;margin-top:0;margin-bottom:20px}input,select,textarea{font:inherit inherit inherit}"
			"input{background:rgba(0,0,0,0);padding:.4em .6em;border:1px solid rgba(0,0,0,.12);border-radius:3px;-webkit-appearance:none;-moz-appearance:none;appearance:none}input:focus{border:1px solid #6E5F57;outline:0}"
			"input[type=submit],input[type=button],button[type=button]{margin:6px;cursor:pointer;line-height:2.6;display:inline-block;padding:0 1.2rem;text-align:center;vertical-align:middle;color:#FFF;border:0;border-radius:3px;background:#6E5F57;-webkit-appearance:none;-moz-appearance:none;appearance:none}"
			".row,.row>div,.row>label{display:block}input[type=submit]:hover{color:#FFF;background:#372F2A}input[type=submit]:focus{outline:0}input[type=file]{width:100%}.row{margin-bottom:14px}"
			".row>label{float:left;width:110px;font-size:14px;position:relative;top:8px}.row>div{margin-left:120px;margin-bottom:12px}.row>div>input{width:100%;display:inline-block}.footer{text-align:right;margin-top:16px}"
			".psrow{text-align: center;}.psrow>div{display:inline-block;margin:10px;}.writemode{color:#ff0000}input[type=\"checkbox\"] {-webkit-appearance: checkbox;appearance: checkbox;}")
		);

	});
	nefry_server.on("/nefry_content", [&]() {
		nefry_server.send(200, "text/css", cssConten);
	});

	nefry_server.on("/reset", [&]() {
		nefry_server.send(200, "text/html", createHtml(F("Nefry Reset"),"",
			F("<h1>Nefry Reset</h1><p>Reset start!</p><a href=\"/\">Back to top</a>")));
		ndelay(2000);
		reset();
	});
	nefry_server.on("/jsform", [&]() {
		String content = F(
			"function jsSubmit(formN){formN.submit();}");
		nefry_server.send(200, "text/javascript", content);
	});
	nefry_server.on("/onreset", [&]() {
		WiFiConf.bootmode = 1;
		delay(10);
		saveConf();
		nefry_server.send(200, "text/html", createHtml(F("Nefry Write mode"),"",
			F("<h1>Nefry Write mode</h1><p>Reset start!</p><a href=\"/\">Back to top</a>")));
		ndelay(2000);
		reset();
	});
	nefry_server.on("/favicon.ico", [&]() {
		nefry_server.send(200, "text/html", "");
	});
}

void Nefry_lib::setupWebCaptivePortal(void) {
	_dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
	nefry_server.onNotFound([&]() {
		nefry_server.send(200, "text/html", createHtml(F("CaptivePortal"),(String)F("<meta http-equiv=\"Refresh\" content=\"10; URL = http://")+ ipaddressStr(WiFi.localIP()) +(String)F("\">"),
			(String)F("<h1>Move to main page!</h1><p>このままの画面で動作させた場合、<br>予期しない動作をする可能性があります。<br>できれば、別ブラウザで開くことを推奨します。<br>そのときブラウザに『192.168.4.1』とURL欄に入力してください。</p><p>Please wait...10sec</p><a href=\"http://")+ ipaddressStr(WiFi.localIP()) +(String)F("\">Move to main page!</a>")));
	});
}

void Nefry_lib::setupWebCss(void) {
	cssAdd("moduleid", WiFiConf.module_id);
}



void Nefry_lib::setupWeb(void) {
	nefry_server.begin();
	setupWebModuleConf();
	setupWebLocalUpdate();
	setupWebOnlineUpdate();
	setupWebWiFiConf();
	setupWebCaptivePortal();
	setupWebConsole();
	setupWebMain();
	setupWebCss();

}

void Nefry_lib::cssAdd(const char* id, String data, bool afterflg) {
	String sCss = ".";
	sCss += id;
	if (afterflg)sCss += F(":after");
	else sCss += F(":before");
	sCss += F("{content:\"");
	sCss += data;
	sCss += F("\"}");
	cssConten += sCss;
}


//main 

void Nefry_lib::nefry_init() {
	Serial.begin(115200);
	setupModule();
	Serial.println(F("\n\nNefry Startup"));
	delay(10);
	push_sw_();
	setLed(0x00, 0x6f, 0x00);
	// scan Access Points
	printModule();
	Serial.println(F("WiFi Startup"));
	setupWifi();
	printIpaddress();
	setLed(0x00, 0x8f, 0x00);
	push_sw_();
	// setup Web Interface
	Serial.println(F("\nServer started"));
	setupWeb();
	push_sw_();
	println("WiFi List");
	println(listWifi());
	setLed(0x00, 0xaf, 0x00);
	push_sw_();
	setLed(0x00, 0xff, 0xff);
	delay(1000);
	if (pushSW_flg == 1) {
		WiFi.softAP(WiFiConf.module_id);
		for (int i = 0; i < 20; i++)
			setConfHtmlPrint(1, i);
		println(F("Nefry Write mode"));
		setLed(0x0f, 0xff, 0xff);
		cssAdd("writemode", F("Write Mode"));
	}
}

int _locp = 0;
void Nefry_lib::nefry_loop() {
	_dnsServer.processNextRequest();
	nefry_server.handleClient();
	if (_locp == 100) {
		searchWiFi();
		_locp = 0;
	}
	_locp++;
}

void Nefry_lib::setupModule(void) {
	if (boardId == 1)
		beginLed(1, 0, NEO_RGB);
	else
		beginLed(1, 0, NEO_GRB);
	delay(1);
	Serial.println(F("\n\nStartup"));
	setLed(0x00, 0x0f, 0x00);
	ESP.wdtDisable();
	ESP.wdtEnable(60000);
	pinMode(4, INPUT_PULLUP);
	setLed(0x00, 0x2f, 0x00);
	push_sw_();
	module_set();
	nefry_server = ESP8266WebServer(80);
	Serial.println(F("server On"));
	Serial.println(sizeof(WiFiConf));
	EEPROM.begin(1534);
	setLed(0x00, 0x4f, 0x00);
	if (!loadConf()) {
		resetModule();
		saveConf();
	}
	Serial.println(F("memory Ok"));
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
void Nefry_lib::print(String text, int ln) {
	if (printcun >= max_console)printcun = 0;
	Serial.print(text);
	if (ln == 1)text += "<br>";
	text.toCharArray(printweb[printcun++], 50);
	if (mojicount <= max_console)mojicount++;
}

void Nefry_lib::println(String text) {
	print(text, 1);
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
		nefry_server.send(200, "text/html", 
			createHtml("Nefry Console",F("<script type=\"text/javascript\" src=\"consolejs\"></script>"),
				F("<h1>Nefry Console</h1>"
			"<p>It can be used as a terminal.</p>"
			"<form  name='msg' method='post' action='console'><div class=\"row\"> <label for=\"console\">console:</label> <div> <input name=\"console\" id='cos' maxlength=\"100\" value=\"\"></div></div>"
			"<div class=\"footer\"><input type='button' value='Send' onclick='pushDoc();' /></div></form>"
			"<div><div id=\"ajaxDiv\"></div><div class=\"row\"><button type=\"button\" onclick=\"loadDoc()\">reload</button>"
			"<button type = \"button\" onclick=\"reload(500);\">0.5sec reload</button>"
			"<button type = \"button\" onclick=\"reload(2000);\">2sec reload</button>"
			"<button type = \"button\" onclick=\"reload(5000);\">5sec reload</button>"
			"<button type = \"button\" onclick=\"clearInterval(timer);\">stop</button></div>"
			"<div class=\"row\"><button type = \"button\" onclick=\"cclear();\">Clear</button></div>"
			"</div><br><a href=\"/\">Back to top</a>")));
	});
	nefry_server.on("/consolejs", [&]() {
		String content = F(
			"  if (window.XMLHttpRequest) {\n"
			"    xmlhttp = new XMLHttpRequest();\n"
			"  } else {\n"
			"    xmlhttp = new ActiveXObject(\"Microsoft.XMLHTTP\");\n"
			"  }\n"
			"  function pushDoc() {\n"
			"  if (window.XMLHttpRequest) {\n"
			"    xmlhttp2 = new XMLHttpRequest();\n"
			"  } else {\n"
			"    xmlhttp2 = new ActiveXObject(\"Microsoft.XMLHTTP\");\n"
			"  }\n"
			"  xmlhttp2.open(\"POST\",\"console\",true);\n"
			"	xmlhttp2.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');\n"
			"	xmlhttp2.send('console='+document.msg.console.value);\n"
			"	document.getElementById('cos').value='';"
			"  }\n"
			"  function loadDoc() {\n"
			"  xmlhttp.onreadystatechange = function() {\n"
			"    if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {\n var newtext=xmlhttp.responseText;if(newtext.match(/ENDUP/)){clearInterval(timer);}if(newtext.match(/UPDNF/)){clearInterval(timer);alert(\"プログラム更新ページが開かれたため、自動更新を停止しました。更新を終えた場合や更新しない場合Clearを押してから間隔を選択してください。\");}"
			"      document.getElementById(\"ajaxDiv\").innerHTML=newtext;\n"
			"    }\n"
			"  }\n"
			"  xmlhttp.open(\"GET\",\"cons\",true);\n"
			"  xmlhttp.send();\n"
			"}\n"
			"  function cclear() {\n"
			"  xmlhttp.open(\"GET\",\"consc\",true);\n"
			"  xmlhttp.send();\n"
			"}\n"
			"var timer; \n"
			"timer = setInterval(\"loadDoc()\",2000);\n"
			"function reload(time) {\n"
			"  clearInterval(timer); \n"
			"  timer = setInterval(\"loadDoc()\",time);\n"
			"}");
		nefry_server.send(200, "text/html", content);
	});
	nefry_server.on("/cons", HTTP_GET, [&]() {
		String content;
		int i;
		i = printcun;
		if (mojicount < max_console)i = 0;
		for (int j = 0; j < mojicount; j++, i++) {
			if (i > max_console)i = 0;
			content += printweb[i];
		}
		nefry_server.send(200, "text/html", content);
	});
	nefry_server.on("/consc", HTTP_GET, [&]() {
		ClearConsole();
		nefry_server.send(200, "text/html", "");
	});
}

void Nefry_lib::ClearConsole() {
	mojicount = 0;
	printcun = 0;
}

//LED

void Nefry_lib::Nefry_LED_blink(const char r, const char g, const char b, const int wait, const int loop, const char pin) {
	int i = 0;
	while (i < loop) {
		setLed(r, g, b, 122, pin);
		ndelay(wait);
		setLed(r, g, b, 0, pin);
		ndelay(wait);
		i++;
	}
}
void Nefry_lib::beginLed(const int num, const int pin, uint8_t t = NEO_GRB + NEO_KHZ800) {
	_NefryLED[pin] = Adafruit_NeoPixel(num, pin, t);
	_NefryLED[pin].begin();
}

void Nefry_lib::setLed(const int r, const int g, const int b, const char w, const char pin, const int num) {
	_NefryLED[(int)pin].setBrightness(w);
	_NefryLED[(int)pin].setPixelColor(num, r, g, b);
	_NefryLED[(int)pin].show();
}
void Nefry_lib::setLed(const char * _colorStr, const char w, const char pin, const int num) {
	int _color[3];
	for (int i = 0; i < 3; i++) {
		_color[i] = 0;
		_color[i] += 16 * hextonum(_colorStr[i * 2]);
		_color[i] += hextonum(_colorStr[i * 2 + 1]);
	}
	_NefryLED[(int)pin].setBrightness(w);
	_NefryLED[(int)pin].setPixelColor(num, _color[0], _color[1], _color[2]);
	_NefryLED[(int)pin].show();
}
int Nefry_lib::hextonum(char c)
{
	char e;
	int n = 0;
	if ('0' <= c && c <= '9') {
		n = c - '0';
	}
	else if ('a' <= (e = tolower(c)) && c <= 'f') {
		n = e - 'a' + 10;
	}
	return n;
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
	if (boardId == 2)
		sprintf(dst, "CocoaBit-%02x%02x", macAddr[WL_MAC_ADDR_LENGTH - 2], macAddr[WL_MAC_ADDR_LENGTH - 1]);
	else
		sprintf(dst, "Nefry-%02x%02x", macAddr[WL_MAC_ADDR_LENGTH - 2], macAddr[WL_MAC_ADDR_LENGTH - 1]);
}

void Nefry_lib::resetModule(void) {
	uint8_t macAddr[WL_MAC_ADDR_LENGTH];
	WiFi.macAddress(macAddr);
	setDefaultModuleId(WiFiConf.module_id);
	Serial.print(F("Reset Module ID to default: "));
	Serial.println(WiFiConf.module_id);
}

void Nefry_lib::printModule(void) {
	Serial.println();
	Serial.print(F("Name: "));
	Serial.println(WiFiConf.module_id);
	Serial.print(F("Module ID: "));
	Serial.println(WiFiConf.module_id);
}
void Nefry_lib::printIpaddress() {
	Serial.print(F("LAN: "));
	Serial.println(WiFi.localIP());
	Serial.print(F("AP: "));
	Serial.println(WiFi.softAPIP());
}

void Nefry_lib::saveConf(void) {
	Serial.println(F("writing WiFiConf"));
	for (int i = 0; i < 4; i++) {
		WiFiConf.format[i] = wifi_conf_format[i];
	}
	EEPROM.write(0, 0);
	for (unsigned int t = 0; t < sizeof(WiFiConf); t++) {
		EEPROM.write(WIFI_CONF_START + t, *((char*)&WiFiConf + t));
	}
	EEPROM.commit();
	delay(10);
}

bool Nefry_lib::loadConf() {
	Serial.println();
	Serial.println(F("loading WiFiConf"));
	for (unsigned int t = 0; t < sizeof(WiFiConf); t++) {
		*((char*)&WiFiConf + t) = EEPROM.read(WIFI_CONF_START + t);
		Serial.print(EEPROM.read(WIFI_CONF_START + t));
	}
	Serial.println();
	for (int i = 0; i < 4; i++) {
		Serial.print(F("load format : "));
		Serial.print(wifi_conf_format[i]);
		Serial.print(F(" : EEPROM format : "));
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
		Serial.println(F("WiFiConf was not saved on EEPROM."));
		return false;
	}
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

//WiFi

void Nefry_lib::deleteWifi(int id, bool lastdata)
{
	if (id < 0)return;
	if (id >= 5)return;
	WiFiConf.ptssid[id] = 0;
	strcpy(WiFiConf.save_ssid[id], "Nefry");
	strcpy(WiFiConf.save_pwd[id], "Nefry-Wifi");
	if (lastdata == true) {
		sortWifi();
		saveWifi();
	}
}

void Nefry_lib::addWifi(String ssid, String pwd)
{
	sortWifi();
	for (int i = 0; i < 5; i++) {
		if (WiFiConf.ptssid[i] == 0) {
			strcpy(WiFiConf.save_ssid[i], ssid.c_str());
			strcpy(WiFiConf.save_pwd[i], pwd.c_str());
			WiFiConf.ptssid[i] = 1;
			saveConf();
			return;
		}
	}
	deleteWifi(0, true);
	strcpy(WiFiConf.save_ssid[4], ssid.c_str());
	strcpy(WiFiConf.save_pwd[4], pwd.c_str());
	saveConf();
	WiFiConf.ptssid[4] = 1;
}
void Nefry_lib::sortWifi()
{
	for (int i = 0; i < 4; i++) {
		if (WiFiConf.ptssid[i] == 0) {
			int j;
			for (j = i + 1; j < 5; j++) {
				if (WiFiConf.ptssid[j] != 0) {
					strcpy(WiFiConf.save_ssid[i], WiFiConf.save_ssid[j]);
					strcpy(WiFiConf.save_pwd[i], WiFiConf.save_pwd[j]);
					strcpy(WiFiConf.save_ssid[j], "");
					strcpy(WiFiConf.save_pwd[j], "");
					WiFiConf.ptssid[i] = 1;
					WiFiConf.ptssid[j] = 0;
					break;
				}
			}
			if (j == 5)return;
		}
	}
}
String Nefry_lib::listWifi()
{
	String lisWifi = "";
	for (int i = 0; i < 5; i++) {
		if (WiFiConf.ptssid[i] != 0) {
			lisWifi += "ID : ";
			lisWifi += i;
			lisWifi += " SSID : ";
			lisWifi += WiFiConf.save_ssid[i];
			lisWifi += "\n";
		}
	}
	//println("listWifi");
	//println(lisWifi);
	return lisWifi;
}

void Nefry_lib::saveWifi()
{
	saveConf();
}

wl_status_t prevWifiStatus = WL_IDLE_STATUS;
int Nefry_lib::searchWiFi()
{
	wl_status_t wifiStatus = wifiMulti.run();
	if (prevWifiStatus != wifiStatus) {
		prevWifiStatus = wifiStatus;
		if (wifiStatus == WL_CONNECTED) {
			Serial.println("WiFi connected");
			Serial.println("SSID: ");
			Serial.println(WiFi.SSID());
			Serial.println("IP address: ");
			Serial.println(WiFi.localIP());
			return 0;
		}
		else {
			Serial.print(F("WiFi errorCode : "));
			switch (wifiStatus)
			{
			case WL_IDLE_STATUS:
				Serial.println(F("Network Idle"));
				break;
			case WL_NO_SSID_AVAIL:
				Serial.println(F("SSID Not Found"));
				break;
			case WL_SCAN_COMPLETED:
				Serial.println(F("Scan Complete"));
				break;
			case WL_CONNECT_FAILED:
				Serial.println(F("Connect Failed"));
				break;
			case WL_CONNECTION_LOST:
				Serial.println(F("Connection Lost"));
				break;
			case WL_DISCONNECTED:
				Serial.println(F("Disconnected"));
				break;
			default:
				break;
			}
			return 1;
		}
	}
	return 2;
}

void Nefry_lib::setupWifi(void) {
	scanWiFiHtml();
	push_sw_();
	// start WiFi
	WiFi.persistent(false);
	WiFi.mode(WIFI_AP_STA);
	if (strcmp(WiFiConf.sta_ssid, "Nefry") != 0) {
		strcpy(WiFiConf.save_ssid[0], WiFiConf.sta_ssid);
		strcpy(WiFiConf.save_pwd[0], WiFiConf.sta_pwd);
		WiFiConf.ptssid[0] = 1;
		strcpy(WiFiConf.sta_ssid, "Nefry");
	}
	for (int i = 0; i < 5; i++) {
		if (WiFiConf.ptssid[i] != 0) {
			wifiMulti.addAP(WiFiConf.save_ssid[i], WiFiConf.save_pwd[i]);
			println("setup Wifi");
			println(WiFiConf.save_ssid[i]);
			println(WiFiConf.save_pwd[i]);
		}
	}

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
			if (searchWiFi() == 0) {
				println(F("WiFi connected"));
				return true;
			}
			Nefry_LED_blink(0x00, 0x4f, 0x00, 100, 1);
			print(".");
			wait++;
			ndelay(100);
		}
		WiFi.disconnect();
		println(F("Connect timed out"));
		return false;
	}
	return true;
}

void Nefry_lib::scanWiFiHtml(void) {
	int founds = WiFi.scanNetworks();
	Serial.println();
	Serial.println(F("scan done"));
	if (founds == 0) {
		Serial.println(F("no networks found"));
	}
	else {
		Serial.print(founds);
		Serial.println(F(" networks found"));
		network_html = F("<ol>");
		network_list = F("<datalist id=\"network_list\">");
		for (int i = 0; i < founds; ++i) {
			// Print SSID and RSSI for each network found
			Serial.print(i + 1);
			Serial.print(": ");
			Serial.print(WiFi.SSID(i));
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));
			Serial.println(")");
			network_html += F("<li>");
			network_html += WiFi.SSID(i);
			network_html += " (";
			network_html += WiFi.RSSI(i);
			network_html += ")";
			network_html += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
			network_html += F("</li>");
			network_list += F("<option value=\"");
			network_list += WiFi.SSID(i);
			network_list += F("\"></option>");
		}
		network_html += F("</ol>");
		network_list += F("</datalist>");
	}
}
bool Nefry_lib::waitConnected(void) {
	int wait = 0;
	Serial.println();
	Serial.println(F("Waiting for WiFi to connect"));
	if (searchWiFi() == 0) {
		Serial.println(F("WiFi connected"));
		return true;
	}
	return false;
}
Nefry_lib Nefry;