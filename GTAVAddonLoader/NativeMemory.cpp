#include "NativeMemory.hpp"

#include "../../ScriptHookV_SDK/inc/main.h"
#include <Windows.h>
#include <Psapi.h>

#include <array>
#include <vector>

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
	unsigned short modelHashEntries = *reinterpret_cast<PUINT16>(baseFuncAddr + *reinterpret_cast<int*>(baseFuncAddr + 3) + 7);
	int modelNum1 = *reinterpret_cast<int*>(*reinterpret_cast<int*>(baseFuncAddr + 0x52) + baseFuncAddr + 0x56);
	unsigned long long modelNum2 = *reinterpret_cast<PUINT64>(*reinterpret_cast<int*>(baseFuncAddr + 0x63) + baseFuncAddr + 0x67);
	unsigned long long modelNum3 = *reinterpret_cast<PUINT64>(*reinterpret_cast<int*>(baseFuncAddr + 0x7A) + baseFuncAddr + 0x7E);
	unsigned long long modelNum4 = *reinterpret_cast<PUINT64>(*reinterpret_cast<int*>(baseFuncAddr + 0x81) + baseFuncAddr + 0x85);
	unsigned long long modelHashTable = *reinterpret_cast<PUINT64>(*reinterpret_cast<int*>(baseFuncAddr + 0x24) + baseFuncAddr + 0x28);
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

// Thank you, Unknown Modder!
typedef __int64(*GetModelInfo_t)(unsigned int modelHash, int* index);
GetModelInfo_t GetModelInfo = (GetModelInfo_t)MemoryAccess::FindPattern("\x0F\xB7\x05\x00\x00\x00\x00\x45\x33\xC9\x4C\x8B\xDA\x66\x85\xC0\x0F\x84\x00\x00\x00\x00\x44\x0F\xB7\xC0\x33\xD2\x8B\xC1\x41\xF7\xF0\x48\x8B\x05\x00\x00\x00\x00\x4C\x8B\x14\xD0\xEB\x09\x41\x3B\x0A\x74\x54",
																		"xxx????xxxxxxxxxxx????xxxxxxxxxxxxxx????xxxxxxxxxxx");

std::vector<uint8_t> MemoryAccess::GetVehicleModKits(int modelHash) {
	std::vector<uint8_t> modKits;

	int index = 0xFFFF;
	uint64_t modelInfo = GetModelInfo(modelHash, &index);

	if (modelInfo && (*(uint8_t*)(modelInfo + 0x9D) & 0x1F) == 5) {
		uint16_t count = *(uint16_t*)(modelInfo + 0x290);
		for (uint16_t i = 0; i < count; i++) {
			uint8_t modKit = *(uint8_t*)(*(uint64_t*)(modelInfo + 0x288) + i);
			modKits.push_back(modKit);
		}
	}

	return modKits;
}

char *MemoryAccess::GetVehicleGameName(int modelHash) {
	int index = 0xFFFF;
	uint64_t modelInfo = GetModelInfo(modelHash, &index);
	return (char*)(modelInfo + 0x270);

}
char *MemoryAccess::GetVehicleMakeName(int modelHash) {
	int index = 0xFFFF;
	uint64_t modelInfo = GetModelInfo(modelHash, &index);
	return (char*)(modelInfo + 0x27C);
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

uint64_t g_fwTxdStore;
uint32_t g_txdCollectionItemSize;
using Hash = unsigned long;

void MemoryAccess::initTxdStore() {
	uint64_t patternAddr = FindPattern("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\x45\xEC",
									   "xxx????x????xxx");
	g_fwTxdStore = patternAddr + *(int*)(patternAddr + 3) + 7;

	patternAddr = FindPattern("\x48\x03\x0D\x00\x00\x00\x00\x48\x85\xD1\x75\x04\x44\x89\x4D\xF0",
							  "xxx????xxxxxxxxx");
	g_txdCollectionItemSize = *(uint32_t*)((patternAddr + *(int*)(patternAddr + 3) + 7) + 0x14);
}

// Thank you, Unknown Modder!
std::vector<rage::grcTexture *> MemoryAccess::GetTexturesFromTxd(Hash txdHash) {
	std::vector<rage::grcTexture *> vecTextures;

	if (g_fwTxdStore && g_fwTxdStore != 7) {
		uint64_t txds = *(uint64_t*)(g_fwTxdStore + 0x70);
		if (txds) {
			uint16_t size = *(uint16_t*)(g_fwTxdStore + 0x82);
			
			// This doesn't quite work somehow for me.
			//for (uint16_t i = txdHash % (size - 1); i < size - 1; i++) {
			for (uint16_t i = txdHash % (size - 1); i < size - 1; i++) {
				Hash hash = *(Hash*)(txds + i * 8);
				if (hash != txdHash) continue;

				uint16_t index = *(uint16_t*)(txds + i * 8 + 4);
				if (index == -1) break;

				uint64_t pgDictionaryCollection = *(uint64_t*)(g_fwTxdStore + 0x38);
				if (pgDictionaryCollection) {
					rage::pgDictionary* dictionary = *(rage::pgDictionary**)(pgDictionaryCollection + index * g_txdCollectionItemSize);
					if (dictionary) {
						rage::grcTexture** textures = dictionary->textures;
						if (textures) {
							uint16_t count = dictionary->textureCount;
							for (uint16_t j = 0; j < count; j++) {
								if (textures[j] == nullptr) continue;
								vecTextures.push_back(textures[j]);
							}
						}
					}
				}
			}
		}
	}
	return vecTextures;
}
