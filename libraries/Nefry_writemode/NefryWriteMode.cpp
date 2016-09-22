/**
Nefry lib

Copyright (c) 2016 wami

This software is released under the MIT License.
http://opensource.org/licenses/mit-license.php
*/

#include "NefryWriteMode.h"

FUNC_POINTER floop;
FUNC_POINTER fsetup;
NefryWriteMode::NefryWriteMode(){}

NefryWriteMode::NefryWriteMode(FUNC_POINTER _Func_setup, FUNC_POINTER _Func_loop){
	floop = _Func_loop;
	fsetup = _Func_setup;
}

void NefryWriteMode::wsetup(){
	if (fsetup != nullptr) {
		fsetup();
	}
}

void NefryWriteMode::wloop(){
	if (floop != nullptr) {
		floop();
	}
}
