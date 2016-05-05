Arduino core for ESP8266 WiFi chip + Nefry Library
===========================================

This project brings support for ESP8266 chip to the Arduino environment. It lets you write sketches using familiar Arduino functions and libraries, and run them directly on ESP8266, no external microcontroller required.

ESP8266 Arduino core comes with libraries to communicate over WiFi using TCP and UDP, set up HTTP, mDNS, SSDP, and DNS servers, do OTA updates, use a file system in flash memory, work with SD cards, servos, SPI and I2C peripherals.

### Installing with Boards Manager ###

1.6.6�ȏ��Arduino IDE���C���X�g�[�������̂��A���ݒ�� Additional Boards Manager URLs ��Boards manager link����́B
.�c�[�����}�C�R���{�[�h���{�[�h�}�l�[�W���[���J���܂��B
�������ɁhNefry�h�Ɠ��͂��Ă��������ƁAAdditional boards manager URLs �Œǉ�����
URL �������I�Ɍ������ANefry �Əo��̂ŉE���ɂ���hInstall�h���N���b�N���ăC���X�g�[�������܂ł�
�΂炭���҂����������B

#### Available versions

##### Stable version ![](http://arduino.esp8266.com/stable/badge.svg)
Boards manager link: `http://wamisnet.github.io/package_nefry_index.json`

Documentation: [Reference](http://wamisnet.github.io/Nefry_manual.pdf)

### License and credits ###

Arduino IDE is developed and maintained by the Arduino team. The IDE is licensed under GPL.

ESP8266 core includes an xtensa gcc toolchain, which is also under GPL.

Esptool written by Christian Klippel is licensed under GPLv2, currently maintained by Ivan Grokhotkov: https://github.com/igrr/esptool-ck.

Espressif SDK included in this build is under Espressif MIT License.

ESP8266 core files are licensed under LGPL.
Nefry library files are licensed under LGPL.

[SPI Flash File System (SPIFFS)](https://github.com/pellepl/spiffs) written by Peter Andersson is used in this project. It is distributed under MIT license.
