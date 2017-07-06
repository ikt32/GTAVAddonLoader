#pragma once

#include <cstdint>
#include <vector>

class MemoryAccess {
public:
	static uintptr_t FindPattern(const char* pattern, const char* mask);
	static std::array<std::vector<int>, 0x20> GenerateVehicleModelList();
	static std::vector<uint8_t> GetVehicleModKits(int modelHash);
};
