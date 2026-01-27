#ifndef CHEATBAN_COMMAND_HEADER
#define CHEATBAN_COMMAND_HEADER

#include "../ICommand.h"
#include "../ChatCommands.h"
#include "Utils/Utils.h"
#include "../../MainServer.h"
#include <source_location>

namespace Main
{
	namespace Command
	{
		class CheatBan final : public ICommand
		{
		private:
			std::string m_targetPlayerName{};

			bool parseCommand(const std::string& providedCommand) override
			{
				std::smatch match;
				if (std::regex_match(providedCommand, match, this->m_pattern))
				{
					m_targetPlayerName = match[1].str();
					return true;
				}
				return false;
			}

		public:
			explicit CheatBan(const Common::Enums::PlayerGrade requiredGrade)
				: ICommand{ requiredGrade, "/cheatban <nickname>: bans a cheater identified by nickname, for an unlimited time"
				, R"(^\S+\s+(.*)$)" }
			{
			}

			void execute(const std::string& command, std::shared_ptr<Main::Network::Session> session, MN::SessionsManager& sessionsManager, MC::RoomsManager&,
				MP::MainScheduler& scheduler, std::uint32_t,
				Main::MainServer&) override
			{
				if (!parseCommand(command))
				{
					session->sendMessage("parsing error");
					return;
				}

				auto targetSession = sessionsManager.findSessionByName(m_targetPlayerName.c_str());
				if (!targetSession)
				{
					if (!scheduler.immediatePersist(std::source_location::current(), &Main::Persistence::PersistentDatabase::updateSuspension, m_targetPlayerName, "",
						"AUTOMATIC_CHEAT_BAN",
						session->getAccountInfo().playerGrade))
					{
						session->sendMessage("error: the player was either not found in the database, or they have equal or higher grade than you");
						return;
					}
				}
				else
				{
					if (targetSession->getAccountInfo().playerGrade >= session->getAccountInfo().playerGrade)
					{
						session->sendMessage("error: The target user has equal or greater grade than you.");
						return;
					}
					if (!targetSession->banAccount(9999, "AUTOMATIC_CHEAT_BAN", true))
					{
						session->sendMessage("error: unknown error");
						return;
					}
				}
				session->sendMessage("success");
			}
		};

		REGISTER_CMD(CheatBan, Common::Enums::PlayerGrade::GRADE_MOD)
	};
}


#endif
