#ifndef NefryWriteMode_h
#define NefryWriteMode_h
#include "NefryWriteModeaw.h"
#include<Arduino.h>
typedef void(*FUNC_POINTER)(void); //ä÷êîÉ|ÉCÉìÉ^ÇÃtypedef

class NefryWriteMode
{
public:
	NefryWriteMode();
	NefryWriteMode(FUNC_POINTER _Func_setup, FUNC_POINTER _Func_loop);
	void wsetup(),
		wloop();
private:
 
};
//extern NefryWriteMode NefryWriteMode;
#endif
