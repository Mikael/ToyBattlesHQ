#ifndef PLAYER_INFO_DB_H
#define PLAYER_INFO_DB_H

#include "AuthEnums.h"
#include <chrono>
#include <format>
#include <string>
#include <utility>
#include "../include/Network/Packet.h"
#include <iostream>
#include "Structures/AuthAccountInfo.h"


#include <mariadb/conncpp.hpp>
#include <mariadb/conncpp/Driver.hpp>
#include <mariadb/conncpp/Connection.hpp>


namespace Auth
{
	namespace Persistence
	{
		class PersistentDatabase
		{
		private:
			sql::Connection* con;  

			std::pair<Common::Network::Packet, Auth::Structures::BasicAccountInfo> 
				getPlayerInfo(const std::string& username, const std::string& password, bool is2fa);
			std::pair<Common::Network::Packet, Auth::Structures::BasicAccountInfo>
				twoFactorLogin(const std::string& username, const std::string& password, const std::string& nonHashedPassword,
					const std::string& secret, bool isLoginOk);

		public:
			PersistentDatabase();
			void addHash(std::uint32_t accountID, std::uint32_t key);
			std::pair<Common::Network::Packet, Auth::Structures::BasicAccountInfo> getPlayerInfo(const std::string& username, const std::string& password);
			bool updateLastLoggedNow(std::uint32_t aid, const std::string& ip);

			~PersistentDatabase()
			{
				if (con) 
				{
					delete con;
				}
			}
		};
	}
}
#endif