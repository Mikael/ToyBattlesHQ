
#ifndef AC_COMMUNICATOR_HEADER
#define AC_COMMUNICATOR_HEADER

#include "Cryptography.h"

#include <vector>
#include <mutex>
#include <cstdint>
#include <cstring>
#include "Packet.h"
#include <string>

#include <ctime>
#include <winsock.h>

#include <array>
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
    class ACCommunicator
    {
    private:
        MVApi::Packet m_packet;
        std::string m_hwid{ "" };

        ACCommunicator() = default;

    public:
        ACCommunicator(const ACCommunicator&) = delete;
        ACCommunicator& operator=(const ACCommunicator&) = delete;

        static ACCommunicator& instance()
        {
            static ACCommunicator Instance;
            return Instance;
        }

        inline std::string getCpuId()
        {
            int cpuInfo[4] = {};
            __cpuid(cpuInfo, 0);

            char vendor[13];
            std::memcpy(vendor + 0, &cpuInfo[1], 4); // EBX
            std::memcpy(vendor + 4, &cpuInfo[3], 4); // EDX
            std::memcpy(vendor + 8, &cpuInfo[2], 4); // ECX
            vendor[12] = '\0';

            return std::string(vendor);
        }

        inline std::string getVolumeSerial()
        {
            DWORD serial = 0;
            if (!GetVolumeInformationA("C:\\", nullptr, 0, &serial, nullptr, nullptr, nullptr, 0))
                return "00000000";

            std::ostringstream oss;
            oss << std::hex << std::uppercase << serial;
            return oss.str();
        }

        inline std::string getMacAddress()
        {
            IP_ADAPTER_INFO adapters[16] = {};
            DWORD buflen = sizeof(adapters);

            if (GetAdaptersInfo(adapters, &buflen) != NO_ERROR)
                return "000000000000";

            for (PIP_ADAPTER_INFO a = adapters; a; a = a->Next)
            {
                if (a->Type == MIB_IF_TYPE_ETHERNET && a->AddressLength == 6)
                {
                    std::ostringstream oss;
                    for (UINT i = 0; i < 6; ++i)
                        oss << std::hex << std::setw(2) << std::setfill('0')
                        << static_cast<int>(a->Address[i]);
                    return oss.str();
                }
            }

            return "000000000000";
        }

        inline std::string generateHwid()
        {
            std::string cpu = getCpuId();
            std::string vol = getVolumeSerial();
            std::string mac = getMacAddress();

            return cpu + "-" + vol + "-" + mac;
        }

        std::size_t safe_min(std::size_t a, std::size_t b)
        {
            return (a < b) ? a : b;
        }

        void saveHwid()
        {
            m_hwid = generateHwid();
        }

        void sendHwid(SOCKET s)
        {
            char hwid[64]{};
            std::memcpy(hwid, m_hwid.c_str(), safe_min(m_hwid.size(), sizeof(hwid) - 1));

            m_packet.setupDefaultCrypt();

            m_packet.setTcpHeader(0, 3);
            m_packet.setCommand(81, 0, 0, 0);
            m_packet.setData(reinterpret_cast<const std::uint8_t*>(hwid), sizeof(hwid));

            auto data = m_packet.generateOutgoingDefaultPacket();
            MVApi::HookManager::sendPacket(reinterpret_cast<const char*>(data.data()), data.size(), s);
        }
    };
}

#endif

