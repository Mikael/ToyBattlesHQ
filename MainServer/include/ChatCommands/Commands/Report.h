#ifndef REPORT_COMPLEXCOMMAND_HEADER
#define REPORT_COMPLEXCOMMAND_HEADER

#include "../ICommand.h"
#include "../ChatCommands.h"
#include "Utils/Utils.h"
#include "Utils/Constants.h"
#include "../../MainServer.h"
#include "../../Classes/ReportManager.h"
#include <source_location>

namespace Main
{
	namespace Command
	{
		class Report final : public ICommand
		{
		private:
			std::string m_targetPlayername{};
			std::string m_reportReason{};

			bool parseCommand(const std::string& providedCommand) override
			{
				std::smatch match;
				if (std::regex_match(providedCommand, match, this->m_pattern))
				{
					m_targetPlayername = match[2].str();
					m_reportReason = match[3].str();
					return true;
				}
				return false;
			}

		public:
			Report(const Common::Enums::PlayerGrade requiredGrade)
				: ICommand{ requiredGrade, "/report <nickname> <reason>"
				, R"(^(\S+)\s*(\S+)\s*(.*)$)" }
			{
			}

			void execute(const std::string& command, std::shared_ptr<Main::Network::Session> session, MN::SessionsManager& sessionsManager,
				MC::RoomsManager& roomsManager,
				MP::MainScheduler& scheduler, std::uint32_t roomNumber,
				Main::MainServer& mainServer) override
			{
				START_BENCHMARK
					if (!parseCommand(command))
					{
						session->sendMessage("error: parsing error, see /? for command usage");
						return;
					}

				if (m_reportReason.empty())
				{
					session->sendMessage("error: report reason cannot be empty");
					return;
				}

				if (m_reportReason.size() >= Common::Constants::maxMailboxMessage)
				{
					session->sendMessage("error: report reason should be less than 255 characters");
					return;
				}

				auto targetSession = sessionsManager.findSessionByName(m_targetPlayername.c_str());
				if (!targetSession)
				{
					session->sendMessage("error: target player not found or not online");
					return;
				}

				const auto& reporterInfo = session->getAccountInfo();
				const auto& targetInfo = targetSession->getAccountInfo();

				Common::Enums::PlayerGrade targetGrade = static_cast<Common::Enums::PlayerGrade>(targetInfo.playerGrade);
				if (targetGrade == Common::Enums::PlayerGrade::GRADE_ES ||
					targetGrade == Common::Enums::PlayerGrade::GRADE_MOD ||
					targetGrade == Common::Enums::PlayerGrade::GRADE_TESTER ||
					targetGrade == Common::Enums::PlayerGrade::GRADE_GM)
				{
					session->sendMessage("error: you cannot report staff members or high-ranking players");
					return;
				}

				std::uint32_t currentRoomNumber = session->getPlayer().getRoomNumber();
				std::string roomInfo = currentRoomNumber > 0 ?
					"Room ID: " + std::to_string(currentRoomNumber) :
					"Lobby";

				Main::Structures::ReportInfo reportInfo(
					0,
					reporterInfo.accountID,
					targetInfo.accountID,
					std::string(reporterInfo.nickname),
					std::string(targetInfo.nickname),
					m_reportReason,
					currentRoomNumber
				);

				std::uint32_t reportId = mainServer.getReportManager().addReport(reportInfo);

				std::string reportMessage = "[REPORT #" + std::to_string(reportId) + "] " +
					std::string(reporterInfo.nickname) +
					" reported " + std::string(targetInfo.nickname) +
					" in " + roomInfo +
					". Reason: " + m_reportReason;

				const auto& allSessions = sessionsManager.getAllSessions();
				std::uint32_t moderatorsNotified = 0;

				for (const auto& sessionPair : allSessions)
				{
					auto modSession = sessionPair.second;
					const auto& modAccountInfo = modSession->getAccountInfo();

					if (modAccountInfo.playerGrade == Common::Enums::PlayerGrade::GRADE_MOD ||
						modAccountInfo.playerGrade == Common::Enums::PlayerGrade::GRADE_TESTER ||
						modAccountInfo.playerGrade == Common::Enums::PlayerGrade::GRADE_GM)
					{
						modSession->sendMessage(reportMessage, Main::Enums::TIP);
						modSession->sendMessage("Use /reports to see all reports or /rr " +
							std::to_string(reportId) + " to acknowledge this report", Main::Enums::INFO);
						moderatorsNotified++;
					}
				}

				session->sendMessage("Report submitted successfully (ID: " + std::to_string(reportId) + ")");

				Utils::Logger::log("REPORT: " + std::string(reporterInfo.nickname) + " -> " +
					std::string(targetInfo.nickname) + " | Room: " + roomInfo +
					" | Reason: " + m_reportReason, Utils::LogType::Info, "ReportCommand");

				END_BENCHMARK(Report::execute, session)
			}
		};

		REGISTER_CMD(Report, Common::Enums::PlayerGrade::GRADE_NORMAL)
	}
}
#endif