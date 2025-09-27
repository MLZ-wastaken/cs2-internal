#include <Windows.h>
#include "gui/renderer.hpp"

BOOL __stdcall DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, GUI::MainThread, hModule, 0, nullptr);
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
