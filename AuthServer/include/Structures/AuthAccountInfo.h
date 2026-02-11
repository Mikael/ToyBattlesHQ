#ifndef AUTH_ACCOUNTINFO_H
#define AUTH_ACCOUNTINFO_H

#include <cstdint>
#include "Macros.h"

namespace Auth
{
	namespace Structures
	{
		// This is sent to the client
		struct BasicAccountInfo
		{
PACK_PUSH(1)
			struct AinfoClient
			{
				std::uint32_t accountId{};
				std::uint32_t hashKey{};
				std::uint32_t level{};
				std::uint32_t exp{};
				std::uint32_t kills{};
				std::uint32_t deaths{};
				std::uint32_t assists{};
				std::uint32_t wins{};
				std::uint32_t losses{};
				std::uint32_t draws{};
				char playerName[16]{};
				std::uint16_t clanIconFrontID{};
				std::uint16_t clanIconBackID{};
				char clanName[20]{};
				std::uint32_t vipInfo{};  // different clients have this
			} ainfoClient;				  // Internal struct is what is sent to the client
PACK_POP()
			std::uint32_t grade{};        // Not sent to client, only used by the server
			std::string secret{};         // Not sent to client, only used by the server
			std::string suspendedUntil{}; // Not sent to client, only used by the server
			std::string hashedPassword{}; // Not sent to client, only used by the server
		};

		struct LoginWrongAttempts
		{
			std::uint32_t totalWrongPasswords{};
			std::uint32_t totalWrong2fas{};
		};

		struct Parsed2faLogin 
		{
			std::string username;
			std::optional<std::string> token;
		};
	}
}

#endif