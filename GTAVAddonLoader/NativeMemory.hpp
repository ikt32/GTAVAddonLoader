#pragma once

#include <cstdint>
#include <vector>
#include "inc/natives.h"

namespace rage {
	class grcTexture {
	public:
		void* VTable; // 0x0000
		char _0x0008[0x20]; // 0x0008
		char* name; // 0x0028
		char _0x0030[0x14]; // 0x0030
		uint32_t unk_0x0044; // 0x0044
		char _0x0048[0x8]; // 0x0048
		uint16_t resolutionX; // 0x0050
		uint16_t resolutionY; // 0x0052
		char _0x0054[0xC]; // 0x0054
		grcTexture* previous; // 0x0060
		grcTexture* next; // 0x0068
		char _0x0070[0x20]; // 0x0070
	};

	class pgDictionary {
	public:
		char _0x0000[0x30]; // 0x0000
		grcTexture** textures; // 0x0030
		uint16_t textureCount; // 0x0038
	};
}


class MemoryAccess {
public:
	static uintptr_t FindPattern(const char* pattern, const char* mask);
	static void initTxdStore();
	static std::vector<rage::grcTexture *> GetTexturesFromTxd(Hash txdHash);
	static std::array<std::vector<int>, 0x20> GenerateVehicleModelList();
	static std::vector<uint8_t> GetVehicleModKits(int modelHash);
};
