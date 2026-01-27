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
	namespace Handlers
	{
		inline void handleServerChannelsInfo(const Common::Network::Packet& request, std::shared_ptr<Common::Network::Session> session)
		{
			std::thread([request, session]() {
				auto playersPerServer = Auth::Utils::getPlayersPerServer(session->getAccountId(), session->getIp());

				using Status = Auth::Enums::ChannelStatus;

				std::vector<std::uint32_t> channels(6, Status::OFFLINE);
				for (const auto& [serverNumber, totalPlayers] : playersPerServer)
				{
					if (serverNumber >= 1 && serverNumber <= 9)
					{
						if (totalPlayers <= 20) channels[serverNumber - 1] = Status::LOW_TRAFFIC;
						else if (totalPlayers <= 100) channels[serverNumber - 1] = Status::MEDIUM_TRAFFIC;
						else if (totalPlayers <= Common::Constants::maxServerCapacity) channels[serverNumber - 1] = Status::HIGH_TRAFFIC;
					}
				}

				Auth::Structures::ChannelsInfo channelsInfo(channels);
				Common::Network::Packet response;
				response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
				response.setCommand(23, 0, 0, static_cast<std::uint8_t>(channels.size()));
				response.setData(reinterpret_cast<std::uint8_t*>(channelsInfo.channels.data()), sizeof(std::uint32_t) * channels.size());
				session->asyncWrite(response);
				}).detach();
		}
	}
}
#endif