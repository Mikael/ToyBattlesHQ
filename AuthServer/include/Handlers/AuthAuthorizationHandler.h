#ifndef AUTH_AUTHORIZATION_HANDLER_H
#define AUTH_AUTHORIZATION_HANDLER_H

#include <string>
#include "Network/Session.h"
#include "Network/Packet.h"
#include "../DbPlayerInfo.h"
#include <AuthService.h>

namespace Auth
{
    namespace Handlers
    {
        std::size_t safe_min(std::size_t a, std::size_t b)
        {
            return (a < b) ? a : b;
        }

        inline void handleAuthUserInformation(const Common::Network::Packet& request, std::shared_ptr<Common::Network::Session> session, Auth::AuthService& authService)
        {
            const std::string username = std::string(reinterpret_cast<const char*>(request.getData() + 48));
            const std::string password = std::string(reinterpret_cast<const char*>(request.getData() + 4));
            
            auto result = authService.login(username, password, session->getIp(), session->getPort(), session->m_hwid);

            Common::Network::Packet response;
            response.setTcpHeader(request.getSession(), Common::Enums::USER_LARGE_ENCRYPTION);
            response.setCommand(22, 0, 0, 0);

            if (!result)
            {
                response.setExtra(result.error());
            }
            else
            {
                auto accountInfo = result.value();
                response.setOption(accountInfo.grade);
                response.setExtra(Auth::Enums::Login::SUCCESS);
                response.setData(reinterpret_cast<std::uint8_t*>(&accountInfo.ainfoClient), sizeof(accountInfo.ainfoClient));
            }
            session->asyncWrite(response);
        }

        inline void handleHwidRetrieval(const Common::Network::Packet& request, std::shared_ptr<Common::Network::Session> session)
        {
            char hwid[64]{};
            std::memcpy(hwid, request.getData(), std::min(request.getDataSize(), 64u));
            session->m_hwid = hwid;
        }
    }
}

#endif