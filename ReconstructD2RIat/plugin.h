#pragma once

#include "pluginmain.h"

typedef LONG(NTAPI* NtSuspendProcess)(IN HANDLE ProcessHAndle);


//functions
bool pluginInit(PLUG_INITSTRUCT* initStruct);
void pluginStop();
void pluginSetup();

// my functions
void suspendProcess(DWORD processId);

// callbacks
PLUG_EXPORT void cbFreezeOnAttach(CBTYPE bType, void* callbackInfo);

// commands callbacks
static bool cbReconstructIAT(int argc, char* argv[]);
