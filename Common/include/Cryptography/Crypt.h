// Add original Author (Qweha on GitHub)

#ifndef CRYPTOGRAPHY_H
#define CRYPTOGRAPHY_H

#include <cstdint>

#define ROTL16(x,y) ((uint16_t)((((uint16_t)(x))<<((y)&15)) | (((uint16_t)(x))>>(16-((y)&15)))))
#define ROTR16(x,y) ((uint16_t)((((uint16_t)(x))>>((y)&15)) | (((uint16_t)(x))<<(16-((y)&15)))))
#define ROTL32(x,y) ((uint32_t)((((uint32_t)(x))<<((y)&31)) | (((uint32_t)(x))>>(32-((y)&31)))))
#define ROTR32(x,y) ((uint32_t)((((uint32_t)(x))>>((y)&31)) | (((uint32_t)(x))<<(32-((y)&31)))))

namespace Common
{
	namespace Cryptography
	{
		struct Crypt
		{
			uint32_t RC5S[26];
			uint32_t RC6S[84];
			uint32_t UserKey = 0;
			bool isUsed = true;

			inline static unsigned char KEY16[16] = { 0x3d, 0x63, 0xc5, 0xa3, 0x6d, 0x9a, 0xdb, 0xa5, 0xd1, 0xb2, 0x7a, 0x17, 0xb6, 0x56, 0x2c, 0xba }; 
			inline static unsigned char KEY32[32] = { 0x76, 0xb7, 0x4b, 0x98, 0x4c, 0x5b, 0xd5, 0xe3, 0xc1, 0x92, 0x33, 0x6a, 0x7b, 0xe6, 0xcc, 0xeb, 0x17, 0x9a, 0x77, 0xbc, 0x31, 0x5d, 0xe7, 0x39, 0xa9, 
				0x32, 0x54, 0x88, 0x66, 0xd3, 0xce, 0x43 };

			Crypt() = default;

			void RC5KeySetup();

			void RC6KeySetup();

			void KeySetup(uint32_t key = 0);
			
			void RC5Encrypt32(const void* source, void* destination, int size);

			void RC5Decrypt32(const void* source, void* destination, int size);

			void RC5Encrypt64(const void* source, void* destination, int size);

			void RC5Decrypt64(const void* source, void* destination, int size);

			void RC6Encrypt128(const void* source, void* destination, int size);

			void RC6Decrypt128(const void* source, void* destination, int size);
		};
	}
}

#endif