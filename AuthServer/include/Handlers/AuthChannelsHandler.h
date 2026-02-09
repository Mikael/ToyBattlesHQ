#ifndef AUTH_CHANNELS_HANDLER_H
#define AUTH_CHANNELS_HANDLER_H

#include <array>
#include <string>
#include <iostream>

#include "Network/Session.h"
#include "Network/Packet.h"
#include "../Structures/AuthChannels.h"
#include "../AuthEnums.h"
#include "Utils/Constants.h"
#include "../AuthUtils.h"

namespace Auth
{
	enum ChannelStatus : std::uint32_t
	{
		LOW_TRAFFIC = 0,			  // [ChannelStatus: 0000000000]
		MEDIUM_TRAFFIC = 0x55555500,  // [ChannelStatus: 1111111111]
		HIGH_TRAFFIC = 0xAAAAAA00,    // [ChannelStatus: 2222222222]
		OFFLINE = 0xFFFFFF00		  // [ChannelStatus: 3333333333]
	};

	namespace Handlers
	{
		inline void handleServerChannelsInfo(const Common::Network::Packet& request, std::shared_ptr<Common::Network::Session> session)
        {
            std::thread([request, session]() {
                auto playersPerServer = Auth::Utils::getPlayersPerServer(session->getAccountId(), session->getIp());
                std::size_t maxChannel = 0;

                for (const auto& [serverNumber, u] : playersPerServer)
                {
                    if (serverNumber > maxChannel) maxChannel = static_cast<std::size_t>(serverNumber);
                }

                std::vector<std::uint32_t> channels(maxChannel);
                for (std::size_t i = 0; i < channels.size(); ++i)
                {
                    channels[i] = static_cast<std::uint32_t>((i + 1) | ChannelStatus::OFFLINE);
                }

                for (const auto& [serverNumber, totalPlayers] : playersPerServer)
                {
                    if (serverNumber < 1) continue; 

                    std::uint32_t status;
                    if (totalPlayers <= 20) status = ChannelStatus::LOW_TRAFFIC;
                    else if (totalPlayers <= 100) status = ChannelStatus::MEDIUM_TRAFFIC;
                    else status = ChannelStatus::HIGH_TRAFFIC;

                    channels[serverNumber - 1] = static_cast<std::uint32_t>(serverNumber | status);
                }

                Common::Network::Packet response;
                response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
                response.setCommand(23, 0, 0, static_cast<std::uint8_t>(channels.size()));
                response.setData(reinterpret_cast<std::uint8_t*>(channels.data()), sizeof(std::uint32_t) * channels.size());

                session->asyncWrite(response);
                }).detach();
        }
	}
}
#endif