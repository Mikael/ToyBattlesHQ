#ifndef CLANS_MANAGER_H
#define CLANS_MANAGER_H

#include <unordered_map>
#include <vector>
#include <optional>
#include "ClanRoom.h"

namespace Main
{
	namespace Classes
	{
		class ClansManager
			{
			private:
				std::unordered_map<std::uint16_t, std::vector<Main::Classes::ClanRoom>> m_clanRoomsByClanId{};

			public:
				void addRoom(const Main::Classes::ClanRoom& room)
				{
					m_clanRoomsByClanId[room.getClanId()].push_back(room);
				}

				bool removeExactRoom(std::uint16_t clanId, std::uint16_t roomNumber)
				{
					auto it = m_clanRoomsByClanId.find(clanId);
					if (it == m_clanRoomsByClanId.end()) return false; 

					auto& clanRooms = it->second;
					auto roomIt = std::find_if(clanRooms.begin(), clanRooms.end(), [roomNumber](const Main::Classes::ClanRoom& room) {
						return room.getRoomNumber() == roomNumber;
						});

					if (roomIt != clanRooms.end())
					{
						roomIt->removeAllPlayers();
						clanRooms.erase(roomIt);
						if (clanRooms.empty())
						{
							m_clanRoomsByClanId.erase(it);
						}
						return true; 
					}
					return false;
				}

				void tryRemovePlayerFromParty(std::uint16_t clanId, std::uint32_t sessionId)
				{
					auto it = m_clanRoomsByClanId.find(clanId);
					if (it == m_clanRoomsByClanId.end()) return;

					auto& clanRooms = it->second;

					for (auto roomIt = clanRooms.begin(); roomIt != clanRooms.end(); ++roomIt)
					{
						auto result = roomIt->removePlayer(sessionId);
						if (!result.has_value())
							continue;

						if (*result)
						{
							clanRooms.erase(roomIt);
						}
						return;
					}
				}

				std::size_t getTotalRooms() const
				{
					std::size_t total = 0;
					for (const auto& pair : m_clanRoomsByClanId)
					{
						total += pair.second.size();
					}
					return total;
				}

				std::vector<Main::Classes::ClanRoom>* getRoomsFor(std::uint16_t clanId)
				{
					auto it = m_clanRoomsByClanId.find(clanId);
					return it != m_clanRoomsByClanId.end() ? &it->second : nullptr;
				}

				const std::vector<Main::Classes::ClanRoom>* getRoomsFor(std::uint16_t clanId) const
				{
					auto it = m_clanRoomsByClanId.find(clanId);
					return it != m_clanRoomsByClanId.end() ? &it->second : nullptr;
				}

				Main::Classes::ClanRoom* getExactRoomFor(std::uint16_t clanId, std::uint16_t roomNumber)
				{
					auto* clanRooms = getRoomsFor(clanId);
					if (!clanRooms) return nullptr;

					auto it = std::find_if(clanRooms->begin(), clanRooms->end(), [roomNumber](const Main::Classes::ClanRoom& room) {
						return room.getRoomNumber() == roomNumber; });

					if (it != clanRooms->end())
					{
						return &(*it);
					}
					return nullptr; 
				}

				std::vector<Main::Structures::RegisteredClanInfo> getAllRegisteredClans() const
				{
					std::vector<Main::Structures::RegisteredClanInfo> registeredClans;
					for (const auto& [clanId, clanRooms] : m_clanRoomsByClanId)
					{
						for (const auto& clanRoom : clanRooms)
						{
							if (auto info = clanRoom.getInfo(); info && clanRoom.isRegistered())
							{
								registeredClans.emplace_back(*info);
							}
						}
					}
					return registeredClans;
				}

				std::optional<std::uint16_t> getNextAvailableRoomNumberFor(std::uint16_t clanId) const
				{
					const auto* clanRooms = getRoomsFor(clanId);
					if (!clanRooms) return 1;

					for (std::uint16_t i = 1; i <= Common::Constants::maxPartiesPerClan; ++i)
					{
						auto it = std::find_if(clanRooms->begin(), clanRooms->end(), [i](const Main::Classes::ClanRoom& room) 
							{ return room.getRoomNumber() == i; });

						if (it == clanRooms->end())
							return i; 
					}
					return std::nullopt;
				}
			};
	}
}
#endif
