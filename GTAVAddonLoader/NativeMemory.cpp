/*
 * Credits & Acknowledgements:
 * Generating vehicle model list: ScriptHookVDotNet source (drp4lyf/zorg93)
 * Getting vehicle modkit IDs:    Unknown Modder
 * Getting textures from dicts:   Unknown Modder
 * Hooking InitVehicleArchetype:  Unknown Modder
 * Find enable mp vehicles:       drp4lyf/zorg93
 */

#include "NativeMemory.hpp"

#include <Windows.h>
#include <Psapi.h>

#include <array>
#include <vector>
#include <unordered_map>

#include <PolyHook/PolyHook/PolyHook.hpp>

#include "Util/Logger.hpp"
#include "Util/Util.hpp"


typedef CVehicleModelInfo*(*GetModelInfo_t)(unsigned int modelHash, int* index);
GetModelInfo_t GetModelInfo;

uint64_t g_fwTxdStore;
uint32_t g_txdCollectionItemSize;

GlobalTable globalTable;
ScriptTable* scriptTable;
ScriptHeader* shopController;

PLH::Detour* InitVehicleArchetype_detour;

typedef CVehicleModelInfo*(*InitVehicleArchetype_t)(const char*, bool, unsigned int);
InitVehicleArchetype_t InitVehicleArchetype_orig;

extern std::unordered_map<Hash, std::string> g_vehicleHashes;

CVehicleModelInfo* InitVehicleArchetype_hook(const char* name, bool a2, unsigned int a3) {
    g_vehicleHashes.insert({ joaat(name), name });
    return InitVehicleArchetype_orig(name, a2, a3);
}

void initInitVehicleArchetypeHooks() {
    auto addr = MemoryAccess::FindPattern("\xE8\x00\x00\x00\x00\x48\x8B\x4D\xE0\x48\x8B\x11", "x????xxxxxxx");
    if (addr == 0) {
        logger.Write(ERROR, "Couldn't find InitVehicleArchetype");
        return;
    }
    addr = (addr + *(int*)(addr + 1) + 5);
    logger.Write(INFO, "Found InitVehicleArchetype at 0x%llX", addr);

    InitVehicleArchetype_detour = new PLH::Detour();
    InitVehicleArchetype_detour->SetupHook((BYTE*)(addr), (BYTE*)&InitVehicleArchetype_hook);
    InitVehicleArchetype_detour->Hook();
    InitVehicleArchetype_orig = InitVehicleArchetype_detour->GetOriginal<InitVehicleArchetype_t>();
}

void deinitInitVehicleArchetypeHooks() {
    delete InitVehicleArchetype_detour;
}

void MemoryAccess::Init() {
	// init txd store
	auto addr = FindPattern("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\x45\xEC",
		"xxx????x????xxx");
	g_fwTxdStore = addr + *(int*)(addr + 3) + 7;

	addr = FindPattern("\x48\x03\x0D\x00\x00\x00\x00\x48\x85\xD1\x75\x04\x44\x89\x4D\xF0",
		"xxx????xxxxxxxxx");
	g_txdCollectionItemSize = *(uint32_t*)((addr + *(int*)(addr + 3) + 7) + 0x14);

    addr = FindPattern("\x0F\xB7\x05\x00\x00\x00\x00"
        "\x45\x33\xC9\x4C\x8B\xDA\x66\x85\xC0"
        "\x0F\x84\x00\x00\x00\x00"
        "\x44\x0F\xB7\xC0\x33\xD2\x8B\xC1\x41\xF7\xF0\x48"
        "\x8B\x05\x00\x00\x00\x00"
        "\x4C\x8B\x14\xD0\xEB\x09\x41\x3B\x0A\x74\x54",
        "xxx????xxxxxxxxxxx????"
        "xxxxxxxxxxxxxx????xxxxxxxxxxx");
	GetModelInfo = (GetModelInfo_t)(addr);

	// find enable MP cars patterns
	if (findShopController())
		enableCarsGlobal();

}

// ScriptHookVDotNet/zorg93
inline bool bittest(int data, unsigned char index) {
	return (data & (1 << index)) != 0;
}

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
	for (int i = 0; i<0x20; i++) {
		hashes[i] = std::vector<int>();
	}
	for (int i = 0; i < modelHashEntries; i++) {
		for (HashNode* cur = HashMap[i]; cur; cur = cur->next) {
			UINT16 data = cur->data;
			if ((int)data < modelNum1 && bittest(*reinterpret_cast<int*>(modelNum2 + (4 * data >> 5)), data & 0x1F)) {
				UINT64 addr1 = modelNum4 + modelNum3 * data;
				if (addr1) {
					UINT64 addr2 = *reinterpret_cast<PUINT64>(addr1);
					if (addr2) {
						if ((*reinterpret_cast<PBYTE>(addr2 + 157) & 0x1F) == 5) {
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
std::vector<uint8_t> MemoryAccess::GetVehicleModKits(int modelHash) {
	std::vector<uint8_t> modKits;

	int index = 0xFFFF;
	auto modelInfo = GetModelInfo(modelHash, &index);

	if (modelInfo && modelInfo->GetModelType() == 5) {
		uint16_t count = modelInfo->m_modKitsCount;
		for (uint16_t i = 0; i < count; i++) {
			uint8_t modKit = modelInfo->m_modKits[i];
			modKits.push_back(modKit);
		}
	}

	return modKits;
}

char *MemoryAccess::GetVehicleGameName(int modelHash) {
	int index = 0xFFFF;
	auto modelInfo = GetModelInfo(modelHash, &index);
    return modelInfo->m_displayName;
	//return (char*)(modelInfo + 0x270);

}
char *MemoryAccess::GetVehicleMakeName(int modelHash) {
	int index = 0xFFFF;
	auto modelInfo = GetModelInfo(modelHash, &index);
    return modelInfo->m_manufacturerName;
	//return (char*)(modelInfo + 0x27C);
}

uintptr_t MemoryAccess::FindPattern(const char *pattern, const char *mask, const char* startAddress, size_t size) {
	const char* address_end = startAddress + size;
	const auto mask_length = static_cast<size_t>(strlen(mask) - 1);

	for (size_t i = 0; startAddress < address_end; startAddress++) {
		if (*startAddress == pattern[i] || mask[i] == '?') {
			if (mask[i + 1] == '\0') {
				return reinterpret_cast<uintptr_t>(startAddress) - mask_length;
			}
			i++;
		}
		else {
			i = 0;
		}
	}
	return 0;
}

uintptr_t MemoryAccess::FindPattern(const char* pattern, const char* mask) {
	MODULEINFO modInfo = { };
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &modInfo, sizeof(MODULEINFO));

	return FindPattern(pattern, mask, reinterpret_cast<const char *>(modInfo.lpBaseOfDll), modInfo.SizeOfImage);
}

// Thank you, Unknown Modder!
std::vector<rage::grcTexture *> MemoryAccess::GetTexturesFromTxd(Hash txdHash) {
	std::vector<rage::grcTexture *> vecTextures;

	if (g_fwTxdStore && g_fwTxdStore != 7) {
		uint64_t txds = *(uint64_t*)(g_fwTxdStore + 0x70);
		if (txds) {
			uint16_t size = *(uint16_t*)(g_fwTxdStore + 0x82);
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

// from EnableMPCars by drp4lyf
bool MemoryAccess::findShopController() {
	// FindPatterns
	__int64 patternAddr = FindPattern("\x4C\x8D\x05\x00\x00\x00\x00\x4D\x8B\x08\x4D\x85\xC9\x74\x11", "xxx????xxxxxxxx");
	if (!patternAddr) {
		logger.Write(ERROR, "ERROR: finding address 0");
		logger.Write(ERROR, "Aborting...");
		return false;
	}
	globalTable.GlobalBasePtr = (__int64**)(patternAddr + *(int*)(patternAddr + 3) + 7);


	patternAddr = FindPattern("\x48\x03\x15\x00\x00\x00\x00\x4C\x23\xC2\x49\x8B\x08", "xxx????xxxxxx");
	if (!patternAddr) {
		logger.Write(ERROR, "ERROR: finding address 1");
		logger.Write(ERROR, "Aborting...");
		return false;
	}
	scriptTable = (ScriptTable*)(patternAddr + *(int*)(patternAddr + 3) + 7);

	DWORD startTime = GetTickCount();
	DWORD timeout = 10000; // in millis

	// FindScriptAddresses
	while (!globalTable.IsInitialised()) {
		scriptWait(100); //Wait for GlobalInitialisation before continuing
		if (GetTickCount() > startTime + timeout) {
			logger.Write(ERROR, "ERROR: couldn't init global table");
			logger.Write(ERROR, "Aborting...");
			return false;
		}
	}
	
	//logger.Write(INFO, "Found global base pointer " + std::to_string((__int64)globalTable.GlobalBasePtr));

	ScriptTableItem* Item = scriptTable->FindScript(0x39DA738B);
	if (Item == NULL) {
		logger.Write(ERROR, "ERROR: finding address 2");
		logger.Write(ERROR, "Aborting...");
		return false;
	}
	while (!Item->IsLoaded())
		Sleep(100);
	
	shopController = Item->Header;
	//logger.Write(INFO, "Found shopcontroller");
	return true;
}

void MemoryAccess::enableCarsGlobal() {
	for (int i = 0; i < shopController->CodePageCount(); i++) {
		__int64 sigAddress = FindPattern("\x28\x26\xCE\x6B\x86\x39\x03", "xxxxxxx", (const char*)shopController->GetCodePageAddress(i), shopController->GetCodePageSize(i));
		if (!sigAddress) {
			continue;
		}
		//logger.Write(INFO, "Pattern found in codepage " + std::to_string(i) + " at memory address " + std::to_string(sigAddress));
		int RealCodeOff = (int)(sigAddress - (__int64)shopController->GetCodePageAddress(i) + (i << 14));
		for (int j = 0; j < 2000; j++) {
			if (*(int*)shopController->GetCodePositionAddress(RealCodeOff - j) == 0x0008012D) {
				int funcOff = *(int*)shopController->GetCodePositionAddress(RealCodeOff - j + 6) & 0xFFFFFF;
				//DEBUGMSG("found Function codepage address at %x", funcOff);
				for (int k = 0x5; k < 0x40; k++) {
					if ((*(int*)shopController->GetCodePositionAddress(funcOff + k) & 0xFFFFFF) == 0x01002E) {
						for (k = k + 1; k < 30; k++) {
							if (*(unsigned char*)shopController->GetCodePositionAddress(funcOff + k) == 0x5F) {
								int globalindex = *(int*)shopController->GetCodePositionAddress(funcOff + k + 1) & 0xFFFFFF;
								//DEBUGMSG("Found Global Variable %d, address = %llX", globalindex, (__int64)globalTable.AddressOf(globalindex));
								logger.Write(INFO, "Setting Global Variable " + std::to_string(globalindex) + " to true");
								*globalTable.AddressOf(globalindex) = 1;
								logger.Write(INFO, "MP Cars enabled");
								return;
							}
						}
						break;
					}
				}
				break;
			}
		}
		break;
	}
	logger.Write(ERROR, "Global Variable not found, check game version >= 1.0.678.1");
}
