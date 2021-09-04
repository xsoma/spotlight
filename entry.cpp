#include "includes.h"
#include <string>
int __stdcall DllMain(HMODULE self, ulong_t reason, void* reserved) {
    if (reason == DLL_PROCESS_ATTACH) {

#ifndef KOLO
        HANDLE thread = CreateThread(nullptr, 0, Client::init, nullptr, 0, nullptr);
        if (!thread)
            return 0;
        std::string OutputFolder = "C:\\spotlight";
        CreateDirectory( OutputFolder.c_str( ), NULL );
#else
        Client::init(nullptr);
#endif
        return 1;
    }

    return 0;
}