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

#include "Hooking.h"

#include "Util/Logger.hpp"
#include "Util/Util.hpp"

typedef CVehicleModelInfo*(*GetModelInfo_t)(unsigned int modelHash, int* index);
typedef CVehicleModelInfo*(*InitVehicleArchetype_t)(const char*, bool, unsigned int);

GetModelInfo_t GetModelInfo;

uint64_t g_fwTxdStore;
uint32_t g_txdCollectionItemSize;

GlobalTable globalTable;
ScriptTable* scriptTable;
ScriptHeader* shopController;

CallHook<InitVehicleArchetype_t> * g_InitVehicleArchetype = nullptr;

extern std::unordered_map<Hash, std::string> g_vehicleHashes;

int gameVersion = getGameVersion();

CVehicleModelInfo* initVehicleArchetype_stub(const char* name, bool a2, unsigned int a3) {
    g_vehicleHashes.insert({ joaat(name), name });
    return g_InitVehicleArchetype->fn(name, a2, a3);
}

void setupHooks() {
    auto addr = MemoryAccess::FindPattern("\xE8\x00\x00\x00\x00\x48\x8B\x4D\xE0\x48\x8B\x11", "x????xxxxxxx");
    if (!addr) {
        logger.Write(ERROR, "Couldn't find InitVehicleArchetype");
        return;
    }
    logger.Write(INFO, "Found InitVehicleArchetype at 0x%p", addr);
    g_InitVehicleArchetype = HookManager::SetCall(addr, initVehicleArchetype_stub);
}

void removeHooks() {
    if (g_InitVehicleArchetype) {
        delete g_InitVehicleArchetype;
        g_InitVehicleArchetype = nullptr;
    }
}

void MemoryAccess::Init() {
	// init txd store
	auto addr = FindPattern("\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\x45\xEC",
		"xxx????x????xxx");
    if (!addr) {
        logger.Write(ERROR, "Couldn't find g_fwTxdStore");
    }
	g_fwTxdStore = addr + *(int*)(addr + 3) + 7;
    logger.Write(INFO, "Found g_fwTxdStore at 0x%llX", g_fwTxdStore);

	addr = FindPattern("\x48\x03\x0D\x00\x00\x00\x00\x48\x85\xD1\x75\x04\x44\x89\x4D\xF0",
		"xxx????xxxxxxxxx");
    if (!addr) {
        logger.Write(ERROR, "Couldn't find g_txdCollectionItemSize");
    }
    g_txdCollectionItemSize = *(uint32_t*)((addr + *(int*)(addr + 3) + 7) + 0x14);
    logger.Write(INFO, "g_txdCollectionItemSize is 0x%llX", g_txdCollectionItemSize);

    addr = FindPattern("\x0F\xB7\x05\x00\x00\x00\x00"
        "\x45\x33\xC9\x4C\x8B\xDA\x66\x85\xC0"
        "\x0F\x84\x00\x00\x00\x00"
        "\x44\x0F\xB7\xC0\x33\xD2\x8B\xC1\x41\xF7\xF0\x48"
        "\x8B\x05\x00\x00\x00\x00"
        "\x4C\x8B\x14\xD0\xEB\x09\x41\x3B\x0A\x74\x54",
        "xxx????xxxxxxxxxxx????"
        "xxxxxxxxxxxxxx????xxxxxxxxxxx");
    if (!addr) {
        logger.Write(ERROR, "Couldn't find GetModelInfo");
    }
	GetModelInfo = (GetModelInfo_t)(addr);

	// find enable MP cars patterns
	if (findShopController())
		enableCarsGlobal();

}

// Thank you, Unknown Modder!
template < typename ModelInfo >
std::vector<uint16_t> GetVehicleModKits_t(int modelHash) {
    std::vector<uint16_t> modKits;
    int index = 0xFFFF;
    auto* modelInfo = reinterpret_cast<ModelInfo*>(GetModelInfo(modelHash, &index));
    if (modelInfo && modelInfo->GetModelType() == 5) {
        uint16_t count = modelInfo->m_modKitsCount;
        for (uint16_t i = 0; i < count; i++) {
            uint16_t modKit = modelInfo->m_modKits[i];
            modKits.push_back(modKit);
        }
    }
    return modKits;
}

std::vector<uint16_t> MemoryAccess::GetVehicleModKits(int modelHash) {
    if (gameVersion < 38) {
        return GetVehicleModKits_t<CVehicleModelInfo>(modelHash);
    }
    else {
        return GetVehicleModKits_t<CVehicleModelInfo1290>(modelHash);
    }
}

char *MemoryAccess::GetVehicleGameName(int modelHash) {
	int index = 0xFFFF;
	void* modelInfo = GetModelInfo(modelHash, &index);
    if (gameVersion < 38) {
        return ((CVehicleModelInfo*)modelInfo)->m_displayName;
    }
    else {
        return ((CVehicleModelInfo1290*)modelInfo)->m_displayName;
    }
}
char *MemoryAccess::GetVehicleMakeName(int modelHash) {
	int index = 0xFFFF;
	void* modelInfo = GetModelInfo(modelHash, &index);
    if (gameVersion < 38) {
        return ((CVehicleModelInfo*)modelInfo)->m_manufacturerName;
    }
    else {
        return ((CVehicleModelInfo1290*)modelInfo)->m_manufacturerName;
    }
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
	/*for (int i = 0; i < shopController->CodePageCount(); i++) {
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
	}*/
	
	for (int i = 0; i < shopController->CodePageCount(); i++)
	{
		int size = shopController->GetCodePageSize(i);

		if (size)
		{
			if (getGameVersion() >= 46) // 1.0.1604.0
			{
				uintptr_t address = FindPattern("\x2D\x00\x00\x00\x00\x2C\x01\x00\x00\x56\x04\x00\x6E\x2E\x00\x01\x5F\x00\x00\x00\x00\x04\x00\x6E\x2E\x00\x01", "xx??xxxx??xxxxx?xx????xxxx?x", (const char*)shopController->GetCodePageAddress(i), size);

				if (address)
				{
					int globalindex = *(int*)(address + 17) & 0xFFFFFF;
					logger.Write(INFO, "Setting Global Variable " + std::to_string(globalindex) + " to true");
					*globalTable.AddressOf(globalindex) = 1;
					logger.Write(INFO, "MP Cars enabled");
					return;
				}
			}
			else
			{
				uintptr_t address = FindPattern("\x2C\x01\x00\x00\x20\x56\x04\x00\x6E\x2E\x00\x01\x5F\x00\x00\x00\x00\x04\x00\x6E\x2E\x00\x01", "xx??xxxxxx?xx????xxxx?x", (const char*)shopController->GetCodePageAddress(i), size);

				if (address)
				{
					int globalindex = *(int*)(address + 13) & 0xFFFFFF;
					logger.Write(INFO, "Setting Global Variable " + std::to_string(globalindex) + " to true");
					*globalTable.AddressOf(globalindex) = 1;
					logger.Write(INFO, "MP Cars enabled");
					return;
				}
			}
		}
	}
	
	logger.Write(ERROR, "Global Variable not found, check game version >= 1.0.678.1");
}
