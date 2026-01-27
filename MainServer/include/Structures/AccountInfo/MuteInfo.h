#ifndef MAIN_ACCOUNT_MUTEINFO_H
#define MAIN_ACCOUNT_MUTEINFO_H

#include <cstdint>
#include <string>

namespace Main
{
	namespace Structures
	{
		struct MuteInfo
		{
			bool isMuted{};
			std::string reason{};
			std::string mutedBy{};
			std::string mutedUntil{};
		};

		struct BanInfo
		{
			bool isBanned{};
			std::string reason{};
			std::string bannedUntil{};
		};
	}
}

#endif