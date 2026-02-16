#ifndef HOOK_MANAGER_H
#define HOOK_MANAGER_H

#include <winsock2.h>
#include <windows.h>
#include <detours/detours.h>  
#include <vector>
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include "Packet.h"

namespace MVApi
{
    class HookManager
    {
    public:
        typedef int(__stdcall* tSend)(SOCKET, const char*, int, int);
        typedef int(__stdcall* tRecv)(SOCKET, char*, int, int);
        typedef int(__thiscall* tChat)(int thisPtr, wchar_t* String, LPCWCH lpWideCharStr, int a4, char a5, char a6, int a7);

    private:
        static inline MVApi::Packet m_packet;

        static inline tSend m_originalSend = nullptr;
        static inline tRecv m_originalRecv = nullptr;
        static inline tChat m_originalChat = nullptr;

        static inline SOCKET m_mainSocket = INVALID_SOCKET;

    public:
        static int __stdcall sendPacket(const char* buf, int len, SOCKET s);
        static int __stdcall recv(SOCKET s, char* buf, int len, int flags);
        static int __fastcall chatHooked(int thisPtr, int, wchar_t* String, LPCWCH lpWideCharStr, int a4, char a5, char a6, int a7);

        static bool installHooks();
        static void removeHooks();
    };
}
#endif
