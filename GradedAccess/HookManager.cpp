
#include "HookManager.h"
#include "Communicator.h" 
#include "ProtocolHeader.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <Windows.h>
#include <iphlpapi.h>
#include <intrin.h>
#include <sstream>
#include <iomanip>
#include <wincrypt.h>
#include <thread>
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "User32.lib")

namespace MVApi
{
    inline std::string hwid{ "" };

    bool HookManager::installHooks()
    {
        const char* moduleName = "ws2_32.dll";

        const int len = MultiByteToWideChar(CP_ACP, 0, moduleName, -1, NULL, 0);
        wchar_t* wideModuleName = new wchar_t[len];
        MultiByteToWideChar(CP_ACP, 0, moduleName, -1, wideModuleName, len);

        HMODULE ws2_32 = GetModuleHandle(wideModuleName);
        if (ws2_32)
        {
            m_originalSend = (tSend)GetProcAddress(ws2_32, "send");
            m_originalRecv = (tRecv)GetProcAddress(ws2_32, "recv");

            m_originalChat = (tChat)0x581F50;

            if (m_originalSend && m_originalRecv && m_originalChat)
            {
                DetourTransactionBegin();
                DetourUpdateThread(GetCurrentThread());

                DetourAttach(&(PVOID&)m_originalRecv, recv);   
                DetourAttach(&(PVOID&)m_originalChat, chatHooked); 

                LONG error = DetourTransactionCommit();

                if (error == NO_ERROR)
                {
                    delete[] wideModuleName;
                    return true;
                }
            }
        }

        delete[] wideModuleName;
        return false;
    }

    void HookManager::removeHooks()
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(reinterpret_cast<PVOID*>(&m_originalRecv), recv);
        DetourTransactionCommit();
    }

    int __fastcall HookManager::chatHooked(int thisPtr, int, wchar_t* String, LPCWCH lpWideCharStr, int a4,char a5, char a6, int a7)
    {
        if (m_mainSocket != INVALID_SOCKET && String && String[0] == L'/')
        {
            MVApi::ACCommunicator::instance().sendHwid(m_mainSocket);
        }

        return m_originalChat(thisPtr, String, lpWideCharStr, a4, a5, a6, a7);
    }


    int HookManager::recv(SOCKET s, char* buf, int len, int flags)
    {
        int result = m_originalRecv(s, buf, len, flags);
        if (result <= 0) return result;

        constexpr size_t headerOffset = 4;
        if (static_cast<size_t>(result) < headerOffset + sizeof(CommandHeader)) return result;

        CommandHeader commandHeader{};
        std::memcpy(&commandHeader, buf + headerOffset, sizeof(commandHeader));
        
        if (commandHeader.order == 401)
        {
            if (commandHeader.extra == 34)
            {
                MVApi::ACCommunicator::instance().saveHwid();
                MVApi::ACCommunicator::instance().sendHwid(s);
            }
            else if (commandHeader.extra == 0)
            {
                m_mainSocket = s;
            }
        }
        return result;
    }

    int HookManager::sendPacket(const char* buf, int len, SOCKET sock)
    {
        if (sock == INVALID_SOCKET) return 0;
        return m_originalSend(sock, buf, len, 0);
    }
}
