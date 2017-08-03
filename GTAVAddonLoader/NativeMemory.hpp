#pragma once

#include <cstdint>
#include <vector>
#include "inc/natives.h"

class MemoryAccess {
public:
	static uintptr_t FindPattern(const char* pattern, const char* mask);
	static void initTxdStore();
	static std::vector<std::string> GetTexturesFromTxd(Hash txdHash);
	static std::array<std::vector<int>, 0x20> GenerateVehicleModelList();
	static std::vector<uint8_t> GetVehicleModKits(int modelHash);
};

namespace rage {
	class grcTexture {
	public:
		char _0x0000[0x28]; // 0x0000
		char* name; // 0x0028
	};

	class pgDictionary {
	public:
		char _0x0000[0x30]; // 0x0000
		grcTexture** textures; // 0x0030
		uint16_t textureCount; // 0x0038
	};
}
