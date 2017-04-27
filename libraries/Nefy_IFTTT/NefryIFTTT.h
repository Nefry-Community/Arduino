#ifndef NefryIFTTT_h
#define NefryIFTTT_h

#include <WiFiClient.h>
#include <Nefry.h>

class Nefry_IFTTT
{
public:
	bool
		send(String event, String Secretkey, String data1 = "", String data2 = "", String data3 = "");
};
extern Nefry_IFTTT IFTTT;
#endif