#ifndef COMMON_ACHIEVEMENT_ENUMS_H
#define COMMON_ACHIEVEMENT_ENUMS_H

namespace Common
{
	namespace Enums
	{
		enum AchievementType
		{
			// Each value represents the bit position where the achievement starts.
			// For example, these values are represented with the bit '0' here:
			// 00011110 11110111 10111101 11101111 01111011 11111110
			TIER_BATTLE = 0,
			TIER_MELEE = 10,
			TIER_RIFLE = 15,
			TIER_SHOTGUN = 20,
			TIER_SNIPER = 25,
			TIER_MG = 30,
			TIER_BAZOOKA = 35,
			TIER_GRENADE = 40
		};

		constexpr inline std::optional<AchievementType> getAchievementTypeByIndex(std::uint32_t cgd_idx)
		{
			if (cgd_idx == 1) return TIER_BATTLE;
			if (cgd_idx == 2) return TIER_MELEE;
			if (cgd_idx == 3) return TIER_RIFLE;
			if (cgd_idx == 4) return TIER_SHOTGUN;
			if (cgd_idx == 5) return TIER_SNIPER;
			if (cgd_idx == 6) return TIER_MG;
			if (cgd_idx == 7) return TIER_BAZOOKA;
			if (cgd_idx == 8) return TIER_GRENADE;
			return std::nullopt;
		}
	}
}

#endif