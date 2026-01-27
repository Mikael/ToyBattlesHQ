
#include <functional>
#include "../../include/Classes/Room.h"
#include "../../include/Classes/RoomsManager.h"
#include "../../include/Structures/Room/RoomsList.h"
#include "../../include/Classes/RoomNumberManager.h"

namespace Main
{
	namespace Classes
	{
		void RoomsManager::addRoom(Main::Classes::Room&& room)
		{
			m_roomByNumber[room.getRoomNumber()] = std::move(room);
		}

		void RoomsManager::removeRoom(std::uint16_t roomNum, std::uint32_t extra)
		{
			auto it = m_roomByNumber.find(roomNum);
			if (it == m_roomByNumber.end()) return;

			if (it->second.isClanRoom()) 
			{
				Main::Classes::RoomNumberGenerator<Main::Enums::RoomType::Clan>::getInstance().release(roomNum);
			}
			else
			{
				Main::Classes::RoomNumberGenerator<Main::Enums::RoomType::Room>::getInstance().release(roomNum);
			}
			it->second.removeAllPlayers(extra);  
			m_roomByNumber.erase(it);  
		}

		std::size_t RoomsManager::getTotalRooms() const
		{
			return m_roomByNumber.size();
		}

		std::vector<Main::Structures::SingleRoom> RoomsManager::getRoomsList()
		{
			std::vector<Main::Structures::SingleRoom> roomsList;
			roomsList.reserve(m_roomByNumber.size());

			for (auto it = m_roomByNumber.begin(); it != m_roomByNumber.end(); )
			{
				auto& room = it->second;

				if (room.getPlayersSize() == 0)
				{
					room.removeAllPlayers();
					it = m_roomByNumber.erase(it); 
					continue;
				}

				if (room.getRoomNumber() >= Common::Constants::clanRoomNumberStart)
				{
					++it;
					continue;
				}

				roomsList.emplace_back(room.getRoomInfo());
				++it;
			}

			return roomsList;
		}


		Main::Classes::Room* RoomsManager::getRoomByNumber(std::uint16_t roomNumber)
		{
			auto it = m_roomByNumber.find(roomNumber);
			if (it != m_roomByNumber.end())
			{
				return &it->second; 
			}
			return nullptr;
		}
	};
}
