void setup() {
Nefry.setWiifAuto(false);//自動更新を停止させます。
Nefry.addWifi("SSID","Password");//WiFiの保存をします。
Nefry.println(Nefry.getlistWifi());//保存されているWiFiリストを表示します。
Nefry.deleteWifi(1);//保存されているWiFiリストを削除します。
Nefry.println(Nefry.getlistWifi());//保存されているWiFiリストを表示します。
}

void loop() {
}
