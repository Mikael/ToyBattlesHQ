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
#include <expected>

namespace Auth
{
	namespace Persistence
	{
		class PersistentDatabase
		{
		private:
			sql::Connection* con;  

		public:
			PersistentDatabase();
			void addHash(std::uint32_t accountID, std::uint32_t key);
			bool updateLastLoggedNow(std::uint32_t aid, const std::string& ip);
			bool logGameEvent(const std::string& logType, const std::string& message, const std::string& severity);

			std::expected<Auth::Structures::BasicAccountInfo, Auth::Enums::Login> getCompletePlayerInfo(const std::string& username);

			bool removeGradeAndSuspend(std::uint32_t accountId, bool isGraded);
			bool getGradedHwid(std::uint32_t accountId, std::string& outHash, std::string& outSalt) const;
			bool setGradedHwid(std::uint32_t accountId, const std::string& hash, const std::string& salt);
			bool updateCurrentHwid(std::uint32_t accountId, const std::string& hash, const std::string& salt);

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