#ifndef PROTOCOL_HEADERS_H
#define PROTOCOL_HEADERS_H

#include <cstdint>

namespace MVApi
{
#pragma pack(push, 1)
	struct TcpHeader
	{
		std::uint32_t bogus : 4 = 0;          // Useless, probably for padding
		std::uint32_t sessionId : 14 = 0;     // To distinguish receiver
		std::uint32_t size : 11 = 0;          // Total size of packet, header included [min:4; max:2047]
		std::uint32_t crypt : 3 = 0;          // Cryptography type
	};
#pragma pack(pop)


#pragma pack(push, 1)
	struct CommandHeader
	{
		std::uint32_t bogus : 4 = 0;    // Useless, probably for padding
		std::uint32_t mission : 2 = 0;  // More informations [max:4]
		std::uint32_t order : 10 = 0;   // Callback Number [max:1023]
		std::uint32_t extra : 8 = 0;    // Used to elaborate results [max:255]
		std::uint32_t option : 8 = 0;   // Options [max:255]
	};
#pragma pack(pop)
}

#endif
