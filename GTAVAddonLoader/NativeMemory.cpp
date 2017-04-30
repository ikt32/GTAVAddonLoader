#include "NativeMemory.hpp"

#include "../../ScriptHookV_SDK/inc/main.h"
#include <Windows.h>
#include <Psapi.h>

#include <array>
#include <vector>

MemoryAccess::MemoryAccess() {
	const uintptr_t patternAddress = FindPattern(EntityPoolOpcodePattern, EntityPoolOpcodeMask);

	// 3 bytes are opcode and its first argument, so we add it to get relative address to patternAddress. 7 bytes are length of opcode and its parameters.
	sAddressEntityPool = reinterpret_cast<MemoryPool **>(*reinterpret_cast<int *>(patternAddress + 3) + patternAddress + 7);
}

// This is ripped straight from ScriptHookVDotNet/zorg93!
inline bool bittest(int data, unsigned char index)
{
	return (data & (1 << index)) != 0;
}

struct HashNode
{
	int hash;
	UINT16 data;
	UINT16 padding;
	HashNode* next;
};

std::array<std::vector<int>, 0x20> MemoryAccess::GenerateVehicleModelList() {
	uintptr_t address = FindPattern("\x66\x81\xF9\x00\x00\x74\x10\x4D\x85\xC0", "xxx??xxxxx") - 0x21;
	UINT64 baseFuncAddr = address + *reinterpret_cast<int*>(address) + 4;
	modelHashEntries = *reinterpret_cast<PUINT16>(baseFuncAddr + *reinterpret_cast<int*>(baseFuncAddr + 3) + 7);
	modelNum1 = *reinterpret_cast<int*>(*reinterpret_cast<int*>(baseFuncAddr + 0x52) + baseFuncAddr + 0x56);
	modelNum2 = *reinterpret_cast<PUINT64>(*reinterpret_cast<int*>(baseFuncAddr + 0x63) + baseFuncAddr + 0x67);
	modelNum3 = *reinterpret_cast<PUINT64>(*reinterpret_cast<int*>(baseFuncAddr + 0x7A) + baseFuncAddr + 0x7E);
	modelNum4 = *reinterpret_cast<PUINT64>(*reinterpret_cast<int*>(baseFuncAddr + 0x81) + baseFuncAddr + 0x85);
	modelHashTable = *reinterpret_cast<PUINT64>(*reinterpret_cast<int*>(baseFuncAddr + 0x24) + baseFuncAddr + 0x28);
	int vehClassOff = *reinterpret_cast<int*>(address + 0x31);

	HashNode** HashMap = reinterpret_cast<HashNode**>(modelHashTable);
	std::array<std::vector<int>, 0x20> hashes;
	for (int i = 0; i<0x20; i++)
	{
		hashes[i] = std::vector<int>();
	}
	for (int i = 0; i < modelHashEntries; i++)
	{
		for (HashNode* cur = HashMap[i]; cur; cur = cur->next)
		{
			UINT16 data = cur->data;
			if ((int)data < modelNum1 && bittest(*reinterpret_cast<int*>(modelNum2 + (4 * data >> 5)), data & 0x1F))
			{
				UINT64 addr1 = modelNum4 + modelNum3 * data;
				if (addr1)
				{
					UINT64 addr2 = *reinterpret_cast<PUINT64>(addr1);
					if (addr2)
					{
						if ((*reinterpret_cast<PBYTE>(addr2 + 157) & 0x1F) == 5)
						{
							hashes[*reinterpret_cast<PBYTE>(addr2 + vehClassOff) & 0x1F].push_back(cur->hash);
						}
					}
				}
			}
		}
	}
	return hashes;
}


uintptr_t MemoryAccess::FindPattern(const char* pattern, const char* mask) {
	MODULEINFO modInfo = {nullptr};
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &modInfo, sizeof(MODULEINFO));

	const char* start_offset = reinterpret_cast<const char *>(modInfo.lpBaseOfDll);
	const uintptr_t size = static_cast<uintptr_t>(modInfo.SizeOfImage);

	intptr_t pos = 0;
	const uintptr_t searchLen = static_cast<uintptr_t>(strlen(mask) - 1);

	for (const char* retAddress = start_offset; retAddress < start_offset + size; retAddress++) {
		if (*retAddress == pattern[pos] || mask[pos] == '?') {
			if (mask[pos + 1] == '\0') {
				return (reinterpret_cast<uintptr_t>(retAddress) - searchLen);
			}

			pos++;
		}
		else {
			pos = 0;
		}
	}

	return 0;
}

uintptr_t MemoryAccess::GetAddressOfEntity(int Handle) const {
	return *reinterpret_cast<uintptr_t*>(GetAddressOfItemInPool(*sAddressEntityPool, Handle) + 8);
}

uintptr_t MemoryAccess::GetAddressOfItemInPool(MemoryPool* PoolAddress, int Handle) {
	if (PoolAddress == nullptr) {
		return 0;
	}

	const int index = Handle >> 8;
	const int flag = PoolAddress->BoolAdr[index]; // flag should be equal to 2 if everything is ok

	// parity check? (taken from ScriptHookDotNet for IV
	if (flag & 0x80 || flag != (Handle & 0xFF)) {
		return 0;
	}

	return (PoolAddress->ListAddr + index * PoolAddress->ItemSize);
}
