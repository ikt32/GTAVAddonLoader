/*
 * Credits & Acknowledgements:
 * Generating vehicle model list: ScriptHookVDotNet source (drp4lyf/zorg93)
 * Getting vehicle modkit IDs:    Unknown Modder
 * Getting textures from dicts:   Unknown Modder
 * InitVehicleArchetype (Legacy):       Unknown Modder
 * InitVehicleArchetype (Enhanced):     avail
 * Find enable mp vehicles (Legacy):    drp4lyf/zorg93
 * Find enable mp vehicles (Enhanced):  Chiheb-Bacha
 * GetModelInfo (Enhanced):             Chiheb-Bacha
 */

#include "NativeMemory.hpp"

#include "Hooking.h"
#include "Util/Logger.hpp"
#include "Util/Util.hpp"
#include "Util/Versions.h"

#include <Windows.h>
#include <Psapi.h>

#include <chrono>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

typedef CVehicleModelInfo* (*GetModelInfo_t)(unsigned int modelHash, int* index);
typedef CVehicleModelInfo* (*InitVehicleArchetype_t)(const char*, bool, unsigned int);
typedef int64_t(*InitVehicleArchetypeEnhanced_t)(uint32_t a1, const char*);

GetModelInfo_t GetModelInfo;

GlobalTable globalTable;
ScriptTable* scriptTable;
ScriptHeader* shopController;

CCallHook<InitVehicleArchetype_t>* g_InitVehicleArchetype = nullptr;
CCallHook<InitVehicleArchetypeEnhanced_t>* g_InitVehicleArchetypeEnhanced = nullptr;

extern std::unordered_map<Hash, std::string> g_vehicleHashes;
int gameVersion = getGameVersion();

struct SHashNode {
    uint32_t hash;
    uint16_t data;
    uint16_t padding;
    SHashNode* next;
};

static uintptr_t g_modelHashTable = 0;
static uint16_t g_modelHashEntries = 0;
static uintptr_t g_modelNum1 = 0;
static uintptr_t g_modelNum2 = 0;
static uintptr_t g_modelNum3 = 0;
static uintptr_t g_modelNum4 = 0;

CVehicleModelInfo* initVehicleArchetype_stub(const char* name, bool a2, unsigned int a3) {
    g_vehicleHashes.insert({ joaat(name), name });
    return g_InitVehicleArchetype->mFunc(name, a2, a3);
}

int64_t initVehicleArchetypeEnhanced_stub(uint32_t a1, const char* name) {
    g_vehicleHashes.insert({ joaat(name), name });
    return g_InitVehicleArchetypeEnhanced->mFunc(a1, name);
}

void setupHooks() {
    uintptr_t addr = 0;
    if (Versions::IsEnhanced()) {
        addr = MemoryAccess::FindPattern("e8 ? ? ? ? 43 89 44 2c");
    }
    else {
        addr = MemoryAccess::FindPattern("e8 ? ? ? ? 48 8b 4d e0 48 8b 11");
    }
    if (!addr) {
        LOG(Error, "Couldn't find InitVehicleArchetype");
        return;
    }
    LOG(Info, "Found InitVehicleArchetype at {}", (void*)addr);
    if (Versions::IsEnhanced()) {
        g_InitVehicleArchetypeEnhanced = HookManager::SetCall(addr, initVehicleArchetypeEnhanced_stub);
    }
    else {
        g_InitVehicleArchetype = HookManager::SetCall(addr, initVehicleArchetype_stub);
    }
}

void removeHooks() {
    if (g_InitVehicleArchetype) {
        delete g_InitVehicleArchetype;
        g_InitVehicleArchetype = nullptr;
    }
    if (g_InitVehicleArchetypeEnhanced) {
        delete g_InitVehicleArchetypeEnhanced;
        g_InitVehicleArchetypeEnhanced = nullptr;
    }
}

// See https://github.com/Chiheb-Bacha/ScriptHookVDotNetEnhanced/blob/842f3f8c3135ba527f79ad046194a5138d8618ec/source/core/NativeMemory.cs#L5833
CVehicleModelInfo* FindCModelInfoEnhanced(unsigned int modelHash, int* index) {
    if (!g_modelHashTable || !g_modelHashEntries) {
        return nullptr;
    }

    SHashNode** hashTable = (SHashNode**)g_modelHashTable;
    uint16_t bucketIndex = (uint16_t)(modelHash % g_modelHashEntries);

    for (SHashNode* cur = hashTable[bucketIndex]; cur != nullptr; cur = cur->next) {
        if (cur->hash != modelHash)
            continue;

        uint16_t data = cur->data;

        int bitArrayValue = *(int*)(g_modelNum2 + (4 * (data >> 5)));
        if ((bitArrayValue & (1 << (data & 0x1F))) == 0)
            continue;

        if (data >= g_modelNum1)
            continue;

        uintptr_t addr1 = g_modelNum4 + g_modelNum3 * data;
        if (addr1 == 0)
            continue;

        return (CVehicleModelInfo*)(*(uintptr_t*)(addr1));
    }

    return nullptr;
}

void MemoryAccess::initGetModelInfo() {
    if (Versions::IsEnhanced()) {
        initGetModelInfoEnhanced();
    }
    else {
        initGetModelInfoLegacy();
    }
}

void MemoryAccess::initGetModelInfoLegacy() {
    uintptr_t addr;
    if (gameVersion <= Versions::EGameVersion::L_1_0_1868_1_NOSTEAM) {
        addr = FindPattern(
            "0f b7 05 ? ? ? ? "
            "45 33 c9 4c 8b da 66 85 c0 "
            "0f 84 ? ? ? ? "
            "44 0f b7 c0 33 d2 8b c1 41 f7 f0 48 "
            "8b 05 ? ? ? ? "
            "4c 8b 14 d0 eb 09 41 3b 0a 74 54");

        if (!addr) {
            LOG(Error, "Couldn't find GetModelInfo");
        }
        else {
            GetModelInfo = (GetModelInfo_t)(addr);
            return;
        }
    }
    else {
        addr = FindPattern("eb 09 41 3b 0a 74 54");
        if (!addr) {
            LOG(Error, "Couldn't find GetModelInfo (v58+)");
        }
        else {
            addr = addr - 0x2C;
            GetModelInfo = (GetModelInfo_t)(addr);
            return;
        }
    }
    GetModelInfo = nullptr;
}

void MemoryAccess::initGetModelInfoEnhanced() {
    uintptr_t addr = FindPattern("74 ? 49 89 d0 4c 8b 1d");
    if (!addr) {
        LOG(Error, "Couldn't find modelHashTable pattern");
        GetModelInfo = nullptr;
        return;
    }

    g_modelHashTable = *(uintptr_t*)(*(int32_t*)(addr + 8) + addr + 12);
    g_modelHashEntries = *(uint16_t*)(addr + *(int32_t*)(addr - 7) - 3);

    MODULEINFO modInfo = {};
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &modInfo, sizeof(MODULEINFO));
    size_t searchSize = (uintptr_t)modInfo.lpBaseOfDll + modInfo.SizeOfImage - addr;

    addr = FindPattern("3b 05 ? ? ? ? 7d ? 48 8b 0d", (const uint8_t*)addr, searchSize);
    if (!addr) {
        LOG(Error, "Couldn't find modelNum pattern");
        GetModelInfo = nullptr;
        return;
    }

    g_modelNum1 = *(uintptr_t*)(*(int32_t*)(addr + 2) + addr + 6);
    g_modelNum2 = *(uintptr_t*)(*(int32_t*)(addr + 11) + addr + 15);
    g_modelNum3 = *(uintptr_t*)(*(int32_t*)(addr + 48) + addr + 52);
    g_modelNum4 = *(uintptr_t*)(*(int32_t*)(addr + 33) + addr + 37);

    GetModelInfo = FindCModelInfoEnhanced;
}

void MemoryAccess::initEnableCarsGlobal() {
    if (findShopController())
        enableCarsGlobal();
}

void MemoryAccess::Init() {
    initGetModelInfo();
    initEnableCarsGlobal();
}

// Thank you, Unknown Modder!
template < typename ModelInfo >
std::vector<uint16_t> GetVehicleModKits_t(int modelHash) {
    if (!GetModelInfo)
        return {};

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

std::string MemoryAccess::GetVehicleMakeName(int modelHash) {
    if (!GetModelInfo)
        return std::string();

    int index = 0xFFFF;
    void* modelInfo = GetModelInfo(modelHash, &index);
    const char* memName = nullptr;
    if (gameVersion < 38) {
        memName = ((CVehicleModelInfo*)modelInfo)->m_manufacturerName;
    }
    else {
        memName = ((CVehicleModelInfo1290*)modelInfo)->m_manufacturerName;
    }
    return memName ? memName : std::string();
}

uintptr_t MemoryAccess::FindPattern(const std::string& pattern, const uint8_t* startAddress, size_t size) {
    std::vector<int> bytes;
    std::stringstream ss(pattern);
    std::string temp;

    while (ss >> temp) {
        if (temp == "?" || temp == "??") bytes.push_back(-1);
        else bytes.push_back(std::stoi(temp, nullptr, 16));
    }

    for (size_t i = 0; i <= size - bytes.size(); ++i) {
        bool found = true;
        for (size_t j = 0; j < bytes.size(); ++j) {
            if (bytes[j] != -1 && startAddress[i + j] != static_cast<uint8_t>(bytes[j])) {
                found = false;
                break;
            }
        }
        if (found) return reinterpret_cast<uintptr_t>(&startAddress[i]);
    }
    return 0;
}

uintptr_t MemoryAccess::FindPattern(const std::string& pattern) {
    MODULEINFO modInfo = {};
    GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &modInfo, sizeof(MODULEINFO));
    return FindPattern(pattern, reinterpret_cast<const uint8_t*>(modInfo.lpBaseOfDll), modInfo.SizeOfImage);
}

// from EnableMPCars by drp4lyf
// Enhanced by Chiheb-Bacha
// 
bool MemoryAccess::findShopController() {
    uintptr_t patternAddr = 0;
    if (Versions::IsEnhanced()) {
        // https://github.com/Chiheb-Bacha/ScriptHookVDotNetEnhanced/blob/842f3f8c3135ba527f79ad046194a5138d8618ec/source/core/NativeMemory.cs#L2232
        patternAddr = FindPattern("48 03 05 ? ? ? ? 4c 85 c0 0f 84 ? ? ? ? e9");
    }
    else {
        patternAddr = FindPattern("48 03 15 ? ? ? ? 4c 23 c2 49 8b 08");
    }

    if (!patternAddr) {
        LOG(Error, "ERROR: finding script table pattern");
        LOG(Error, "Aborting...");
        return false;
    }

    scriptTable = (ScriptTable*)(patternAddr + *(int*)(patternAddr + 3) + 7);

    const uint32_t shopControllerHash = 0x39DA738B; // joaat("shop_controller")

    auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    ScriptTableItem* scriptItem = nullptr;

    while (!(scriptItem = scriptTable->FindScript(shopControllerHash))) {
        scriptWait(100);
        if (std::chrono::steady_clock::now() > timeout) {
            LOG(Error, "ERROR: shop_controller script not found in script table (timeout)");
            LOG(Error, "Aborting...");
            return false;
        }
    }

    timeout = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (!scriptItem->IsLoaded()) {
        Sleep(100);
        if (std::chrono::steady_clock::now() > timeout) {
            LOG(Error, "ERROR: shop_controller script not loaded (timeout)");
            LOG(Error, "Aborting...");
            return false;
        }
    }

    shopController = scriptItem->Header;
    //LOG(Info, "Found shop_controller script");
    return true;
}

void MemoryAccess::enableCarsGlobal() {
    const std::string patt_enhanced = "2d ? ? ? ? 2c ? ? ? 56 ? ? 71 2e ? ? 62";

    const std::string patt617_1 = "2c 01 00 00 20 56 04 00 6e 2e 00 01 5f ? ? ? ? 04 00 6e 2e 00 01";
    const std::string patt1604_0 = "2d ? ? ? ? 2c 01 00 00 56 04 00 6e 2e 00 01 5f ? ? ? ? 04 00 6e 2e 00 01";
    const std::string patt2802_0 = "2d ? ? ? ? 2c 01 00 00 56 04 00 71 2e 00 01 62 ? ? ? ? 04 00 71 2e 00 01";

    std::string pattern = patt617_1;
    unsigned int offset = 13;

    if (Versions::IsEnhanced()) {
        pattern = patt_enhanced;
        offset = 17;
    }
    else if (getGameVersion() >= 80) {
        pattern = patt2802_0;
        offset = 17;
    }
    else if (getGameVersion() >= 46) {
        pattern = patt1604_0;
        offset = 17;
    }

    for (int i = 0; i < shopController->CodePageCount(); i++) {
        int size = shopController->GetCodePageSize(i);
        if (size) {
            uintptr_t address = FindPattern(pattern, reinterpret_cast<const uint8_t*>(shopController->GetCodePageAddress(i)), size);
            if (address) {
                int globalindex = *(int*)(address + offset) & 0xFFFFFF;
                LOG(Info, "[MP Vehicles] Setting Global Variable {} to true", globalindex);
                auto* globalPtr = getGlobalPtr(globalindex);
                if (globalPtr) {
                    *globalPtr = 1;
                    LOG(Info, "[MP Vehicles] Enabled");
                    return;
                }
            }
        }
    }

    LOG(Error, "Global Variable not found, check game version >= 1.0.678.1");
}
