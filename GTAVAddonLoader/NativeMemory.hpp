// Implementation provided by LeFix

#pragma once

#include <cstdint>
#include <vector>


struct MemoryPool {
	uintptr_t ListAddr;
	char* BoolAdr;
	int MaxCount;
	int ItemSize;
};

class MemoryAccess {
public:
	MemoryAccess();

	uintptr_t GetAddressOfEntity(int Handle) const;
	static uintptr_t FindPattern(const char* pattern, const char* mask);
	std::array<std::vector<int>, 0x20> GenerateVehicleModelList();

private:
	static uintptr_t GetAddressOfItemInPool(MemoryPool* PoolAddress, int Handle);
	const char* EntityPoolOpcodeMask = "xxx????xxxxxxx";
	const char* EntityPoolOpcodePattern = "\x4C\x8B\x0D\x00\x00\x00\x00\x44\x8B\xC1\x49\x8B\x41\x08";
	MemoryPool** sAddressEntityPool = nullptr;

	unsigned long long modelHashTable, modelNum2, modelNum3, modelNum4;
	int modelNum1;
	unsigned short modelHashEntries;
};
