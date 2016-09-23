#ifndef NefryWriteMode_h
#define NefryWriteMode_h
#include "NefryWriteModeaw.h"
typedef void(*FUNC_POINTER)(void); 
class NefryWriteMode
{
public:
	NefryWriteMode();
	NefryWriteMode(FUNC_POINTER _Func_setup, FUNC_POINTER _Func_loop);
	void wsetup(),
		wloop();
private:
 
};
#endif
