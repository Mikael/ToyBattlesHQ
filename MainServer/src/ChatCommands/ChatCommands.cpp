
#include "../../include/ChatCommands/ICommand.h"
#include "../../include/ChatCommands/ChatCommands.h"
#include <algorithm>
#include <string>
#include "../../include/MainServer.h"

namespace Main
{
	namespace Command
	{
		void ChatCommands::addCommand(std::string name, std::unique_ptr<ICommand> command)
		{
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);
			m_commands[name] = std::move(command);
		}

		bool ChatCommands::executeCommand(const std::string& commandName, const std::string& wholeCommand, std::shared_ptr<Main::Network::Session> session,
			MN::SessionsManager& sessionsManager,
			MC::RoomsManager& roomsManager, MP::MainScheduler& scheduler, std::uint32_t roomNumber, Main::MainServer& mainServer)
		{
			if (m_commands.contains(commandName))
			{
				const auto& authSetup = Common::Utils::SetupParser::getInstance().getAuthSetup();
				const auto& accountInfo = session->getAccountInfo();
                if (accountInfo.playerGrade >= Common::Enums::GRADE_MOD && authSetup.enhancedSecurity)
                {
                    auto now = std::chrono::system_clock::now();
                    auto nowSeconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
					asio::ip::network_v4 vpnNet = asio::ip::make_network_v4(authSetup.gradedAccessSubnet);
					std::error_code ec;
					auto clientIp = asio::ip::make_address_v4(session->getIp(), ec);

                    const bool hwidCorrect = (Common::Utils::hashSha256(session->m_hwid, session->m_gradedHwidSalt) == session->m_gradedHwid);
                    const bool updatedRecently = (nowSeconds - session->m_hwidLastUpdatedTimestamp <= 20);
					const bool isIpAllowed = !session->getIp().empty() && !ec && isIpInSubnet(vpnNet, clientIp);
					
                    if (!hwidCorrect || !updatedRecently || !isIpAllowed)
                    {
                        session->closeSocket();
						return false;
                    }
                }
				if (accountInfo.playerGrade < m_commands[commandName]->getRequiredGrade(*m_scheduler)) return false;
				m_commands[commandName]->execute(wholeCommand, session, sessionsManager, roomsManager, scheduler, roomNumber, mainServer);
				return true;
			}
			return false;
		}

		void ChatCommands::showUsages(std::shared_ptr<Main::Network::Session> session, Common::Network::Packet& response, Common::Enums::PlayerGrade playerGrade)
		{
			for (const auto& [unused, commandImpl] : m_commands)
			{
				if (playerGrade < commandImpl->getRequiredGrade(*m_scheduler)) continue;
				commandImpl->sendCommandUsage(session);
			}
		}
	} // namespace Command
} // namespace Main

