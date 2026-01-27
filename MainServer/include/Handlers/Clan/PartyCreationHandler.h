#ifndef CLAN_PARTY_CREATION_HANDLER_H
#define CLAN_PARTY_CREATION_HANDLER_H

#include "../../Network/MainSession.h"
#include "Network/Packet.h"
#include "../../Classes/ClansManager.h"
#include "../../Structures/Clan/ClanStructures.h"

namespace Main
{
	namespace Handlers
	{
		enum ClanMatchCreationExtra
		{
			CLAN_MATCH_CREATE_SUCCESS = 1,
			CLAN_MATCH_CREATE_EMPTY = 6, // "Cannot create ClanBattle"
			CLAN_MATCH_CREATE_FAIL = 7,  // nothing happens, use 6 
		};

        // Reviewed 6/02/2025
        inline void handlePartyCreation(const Common::Network::Packet& request, std::shared_ptr<Main::Network::Session> session, Main::Classes::ClansManager& clansManager)
        {
            Main::ClientData::ClanRoomSettings clientReq = Details::parseData<Main::ClientData::ClanRoomSettings>(request);

            Common::Network::Packet response = request;
            response.setTcpHeader(request.getSession(), Common::Enums::NO_ENCRYPTION);

            if (const std::optional<std::uint16_t> nextRoomNumberOpt = clansManager.getNextAvailableRoomNumberFor(session->getAccountInfo().clanId))
            {
                session->sendMessage("Created party room number: " + std::to_string(*nextRoomNumberOpt), Main::Enums::TIP);

                response.setExtra(ClanMatchCreationExtra::CLAN_MATCH_CREATE_SUCCESS);
                Main::Classes::ClanRoom createdRoom{ session, *nextRoomNumberOpt, clientReq };
                const auto roomIdPair = createdRoom.getRoomId();
                clansManager.addRoom(createdRoom);
                response.setData(reinterpret_cast<const std::uint8_t*>(&roomIdPair), sizeof(roomIdPair));
            }
            else
            {
                session->sendMessage("Error: Only 4 simultaneous clan matches per clan are allowed");
                response.setExtra(ClanMatchCreationExtra::CLAN_MATCH_CREATE_EMPTY);
            }
            session->asyncWrite(response);
        }
	}
}

#endif