/*DRV8830ドライバーを使ってモータを回します。
 * 
 * SDA:D1
 * SCL:D0
 * 
 * -----------------
 * DRV8830 接続例http://akizukidenshi.com/catalog/g/gK-06273/
 * DRV8830---Nefry
 * 
 * SCL---D0
 * SDA---D1
 * A1---GND
 * A0---GND
 * FAULTn---open
 * GND---GND
 * VCC---3.3v
 * OUT1---motor
 * ISENSE---open
 * OUT2---motor
 * 
 */

#include <Wire.h>
#define ADDR  0x60
void setup() {
Wire.begin();
}

void start_motor(){
  Wire.beginTransmission(ADDR);
  Wire.write(0x00);
  Wire.write(0xFD);
  Wire.endTransmission();
}
void stop_motor(){
  Wire.beginTransmission(ADDR);
  Wire.write(0x00);
  Wire.write(0x03);
  Wire.endTransmission();
}

void loop() {
  // モータを回転、停止するプログラム
 start_motor();
 delay(1000);
 stop_motor();
 delay(1000);
}

