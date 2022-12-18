#include "plugin.h"
#include <string>

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

//Initialize your plugin data here.
bool pluginInit(PLUG_INITSTRUCT* initStruct)
{

    _plugin_registercallback(
        pluginHandle,
        CB_ATTACH,
        cbFreezeOnAttach
    );

    if (!_plugin_registercommand(pluginHandle, PLUGIN_NAME, cbReconstructIAT, true))
        _plugin_logprint("[" PLUGIN_NAME "]:: _plugin_registercommand failed.");


    return true; //Return false to cancel loading the plugin.
}

//Deinitialize your plugin data here.
void pluginStop()
{
}

//Do GUI/Menu related things here.
void pluginSetup()
{
}

// my functions

void suspendProcess(DWORD processId)
{
    HANDLE procHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

    if (procHandle == INVALID_HANDLE_VALUE)
    {
        // handle the error
        _plugin_logprintf("[" PLUGIN_NAME "]:: Suspend:: OpenProcess failed to open process with process id: %d\n", processId);
        return;
    }

    NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)::GetProcAddress(::GetModuleHandle("ntdll"), "NtSuspendProcess");

    if (pfnNtSuspendProcess == NULL)
    {
        // handle the error
        _plugin_logprintf("[" PLUGIN_NAME "]:: Suspend:: GetProcAddress failed\n");
        return;
    }

    LONG status = pfnNtSuspendProcess(procHandle);

    if (!NT_SUCCESS(status))
    {
        // handle the error
        _plugin_logprintf("[" PLUGIN_NAME "]:: Suspend:: NtSuspendProcess failed to suspend process with process id: %d\n", processId);
        return;
    }
    
    ::CloseHandle(procHandle);
}


// callbacks

PLUG_EXPORT void cbFreezeOnAttach(CBTYPE bType, void* callbackInfo)
{
    PLUG_CB_ATTACH* cbInfo = (PLUG_CB_ATTACH*)callbackInfo;
    suspendProcess(cbInfo->dwProcessId);
}

static bool cbReconstructIAT(int argc, char* argv[])
{
    if (argc != 3) return false;

    // argv[2] == IAT address
    duint va_iat_addr = std::stoull(argv[1], nullptr, 16);

    // argv[3] == IAT size
    UINT iat_size = std::stoul(argv[2], nullptr, 16);

    // buffer to store the memory
    duint* iat_mem = new duint[iat_size / sizeof(duint)];

    if (!DbgCmdExecDirect("OverwatchDumpFix"))
    {
        _plugin_logprintf("[" PLUGIN_NAME "]:: cbReconstructIAT:: DbgCmdExec failed\n");
        delete[] iat_mem;
        return false;
    }

    if (!DbgMemRead(va_iat_addr, (void*)iat_mem, iat_size))
    {
        _plugin_logprintf("[" PLUGIN_NAME "]:: cbReconstructIAT:: DbgMemRead failed\n");
        delete[] iat_mem;
        return false;
    }

     BridgeList<Script::Module::ModuleInfo> modules;
    
    if (!Script::Module::GetList(&modules))
    {
        _plugin_logprintf("[" PLUGIN_NAME "]:: cbReconstructIAT:: GetList failed\n");
        delete[] iat_mem;
        return false;
    }

    for (size_t i = 0; i < iat_size / sizeof(void*); i++)
    {
        duint value     = iat_mem[i];
        duint candidate = 0;

        for (int j = modules.Count() - 1; j >= 0; j--)
        {
            candidate = 0xffffffff00000000 & modules[j].base;
            candidate |= value & 0x00000000ffffffff;
            
            // make sure that the current candidate is an export of a module.
            if (candidate < modules[j].base + modules[j].size && candidate > modules[j].base)
            {
                BridgeList<Script::Module::ModuleExport> exports;

                if (!Script::Module::GetExports(&modules[j], &exports))
                {
                    _plugin_logprintf("[" PLUGIN_NAME "]:: cbReconstructIAT:: GetExports failed for module: %s\n", modules[j].name);
                    delete[] iat_mem;
                    return false;
                }

                for (size_t k = 0; k < exports.Count(); k++)
                {
                    // the candidate is a valid export address
                    if (exports[k].va == candidate)
                    {
                        if (!DbgMemWrite(va_iat_addr + i * 8, (void*)&candidate, 8))
                        {
                            _plugin_logprintf("[" PLUGIN_NAME "]:: cbReconstructIAT:: DbgMemWrite failed\n");
                            delete[] iat_mem;
                            return false;
                        }
                    }
                }
            }
        }
    }

    delete[] iat_mem;

    return true;

}
