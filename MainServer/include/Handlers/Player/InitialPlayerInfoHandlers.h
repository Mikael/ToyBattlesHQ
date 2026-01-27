#ifndef MAIN_INITIALPLAYERINFO_HANDLER_H
#define MAIN_INITIALPLAYERINFO_HANDLER_H


#include "Network/Session.h"
#include "MainAccountInfoHandler.h"
#include "../EventsHandlers.h"
#include "AuthorizationHandler.h"
#include "../../Detail/Utilities.h"
#include <chrono>

namespace Main
{
	namespace Handlers
	{
		inline void handleInitialPlayerInfos(const Common::Network::Packet& request, std::shared_ptr<Main::Network::Session> session,
			Main::Network::SessionsManager& sessionsManager, Main::Persistence::MainScheduler& scheduler, std::uint64_t timeSinceLastServerRestart,
			std::uint32_t serverId, bool isServerOffline, bool isPublic, const Main::Structures::EventMissionInfo& eventMissionInfo,
			const Main::Structures::ExpMpBonusInfo& expMpBonusInfo, Main::Classes::ReportManager& reportManager)
		{
			if (auto accountInfo = handleAuthorization(request, session, sessionsManager.getAllSessions().size(), isServerOffline, scheduler, isPublic, reportManager); accountInfo)
			{
				handleAccountInformation(request, session, sessionsManager, scheduler, *accountInfo, timeSinceLastServerRestart, serverId);
				session->sendInventory(*reinterpret_cast<const std::uint32_t*>(request.getData()));
				handleAccountInformation(request, session, sessionsManager, scheduler, *accountInfo, timeSinceLastServerRestart, serverId, 59);

				session->sendWeeklyReward();
				session->sendMonthlyReward();

				const std::uint32_t now = static_cast<std::uint32_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
				if (now >= eventMissionInfo.startDate && now <= eventMissionInfo.endDate)
				{
					session->setEventMissions(scheduler.immediatePersist(std::source_location::current(),
						&Main::Persistence::PersistentDatabase::getPlayerMissions, accountInfo->accountID));
				}

				if (now >= expMpBonusInfo.startDate && now <= expMpBonusInfo.endDate)
				{
					Main::Handlers::handleModeEvents(request, session, scheduler);
					Main::Handlers::handleMapEvents(request, session, scheduler);
				}
			}
		}
	}
}


#endif	
