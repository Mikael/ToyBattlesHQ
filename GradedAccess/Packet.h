#ifndef COMMON_PACKET_H
#define COMMON_PACKET_H

#include <stdlib.h>
#include <vector>

#include "ProtocolHeader.h"
#include "Cryptography.h"
#include <optional>

namespace MVApi
{
    class Packet
    {
    private:
        MVApi::Crypt m_defaultCrypt{};

        MVApi::TcpHeader m_header{};
        MVApi::CommandHeader m_command{};
        std::vector<unsigned char> m_data{};

    public:
        Packet() = default;

        void setupDefaultCrypt()
        {
            m_defaultCrypt.KeySetup(0);
        }

        void setCommand(std::uint16_t order, std::uint8_t mission, std::uint8_t extra, std::uint8_t option)
        {
            m_command = MVApi::CommandHeader{ 0, mission, order, extra, option };
        }

        void setTcpHeader(std::uint32_t sessionId, std::uint32_t crypt)
        {
            m_header = MVApi::TcpHeader{ 0, sessionId, static_cast<std::uint32_t>(sizeof(m_header) + sizeof(m_command) + m_data.size()), crypt };
        }

        void setData(const unsigned char* data, uint16_t size)
        {
            m_data.resize(size);
            std::memcpy(m_data.data(), data, size);
            m_header.size = 8 /*header size*/ + size;
        }

        std::vector<unsigned char> generateOutgoingDefaultPacket()
        {
            constexpr std::size_t headerSize = sizeof(MVApi::TcpHeader);
            constexpr std::size_t commandSize = sizeof(MVApi::CommandHeader);
            const std::size_t partialSize = commandSize + m_data.size();

            std::vector<unsigned char> completeData(headerSize + commandSize + m_data.size(), 0);
            std::memcpy(completeData.data(), &m_header, headerSize);
            std::memcpy(completeData.data() + headerSize, &m_command, commandSize);
            std::memcpy(completeData.data() + headerSize + commandSize, m_data.data(), m_data.size());

            m_defaultCrypt.RC5Encrypt32(completeData.data(), completeData.data(), headerSize);
            m_defaultCrypt.RC6Encrypt128(completeData.data() + headerSize, completeData.data() + headerSize, partialSize);

            return completeData;
        }

    };
}

#endi
