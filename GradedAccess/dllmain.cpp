#include <winsock2.h>
#include <windows.h>
#include <detours/detours.h>
#include "HookManager.h"

#pragma comment(lib, "ws2_32.lib")


DWORD WINAPI start(HMODULE hModule) 
{
    MVApi::HookManager::installHooks();
    return 1;
}


// Required by original steam_api.dll
extern "C" __declspec(dllexport) char SteamAPI_Init(char a1) { return 1; }
extern "C" __declspec(dllexport) int SteamFriends(int a1) { return 1; }
extern "C" __declspec(dllexport) int SteamAPI_RunCallbacks(int a1) { return 1; }
extern "C" __declspec(dllexport) int SteamAPI_UnregisterCallback(int a1) { return 1; }
extern "C" __declspec(dllexport) int SteamAPI_Shutdown(int a1) { return 1;}
extern "C" __declspec(dllexport) int SteamAPI_RegisterCallback(int a1, int a2) { return 1; }


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: 
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)start, hModule, 0, nullptr);
    }
    return TRUE;
}
