#include "script.h"

#include "settings.h"
#include "VehicleHashes.h"
#include "ExtraTypes.h"
#include "UserDLC.h"

#include "Util/Logger.hpp"
#include "Util/Paths.h"
#include "Util/Util.hpp"
#include "NativeMemory.hpp"

#include "menu.h"
#include "menucontrols.h"

#include "inc/natives.h"

#include <fstream>
#include <array>
#include <filesystem>
#include <iomanip>
#include <set>
#include <sstream>
#include <vector>
#include <chrono>

namespace fs = std::filesystem;

NativeMenu::Menu menu;
NativeMenu::MenuControls controls;
Settings settings;

std::string settingsGeneralFile;
std::string settingsMenuFile;

// Keep a list of vehicles we marked as mission entity
std::vector<Vehicle> g_persistentVehicles;

// Stock vehicles DLC. Needs to be updated every DLC release. 
std::vector<DLCDefinition> g_dlcs;

// User vehicles DLCs. User-updateable.
std::vector<DLCDefinition> g_userDlcs;

// All vehicles discovered during load. Unfiltered - contains everything.
std::unordered_map<Hash, std::string> g_vehicleHashes;

// Classes and makes, for grouping in the main menu by either class or make.
std::set<std::string> g_addonClasses;   // Grouping-related
std::set<std::string> g_addonMakes;     // Grouping-related
std::set<std::string> g_dlcClasses;     // Grouping-related
std::set<std::string> g_dlcMakes;       // Grouping-related

// Add-on images. Or images, as they're also used for stock vehicles.
std::vector<AddonImage> g_addonImages;

// Vehicles for which the image is missing - display the default "noimage" for these.
std::vector<Hash> g_missingImages;
AddonImage noImage;

// These have been filtered by user DLC
std::vector<ModelInfo> g_addonVehicles;     // add-on vehicles - used for sorting
std::vector<ModelInfo> g_dlcVehicles;       // game vehicles - used for sorting

// These contain everything
std::vector<ModelInfo> g_addonVehiclesAll;     // all add-on vehicles - used for search
std::vector<ModelInfo> g_dlcVehiclesAll;       // all game vehicles - used for search

void clearImages() {
    g_missingImages.clear();
    g_addonImages.clear();
}

/**
 * Resolving images only when we need it. Should take just 1 tick after an option is selected.
 * Only runs once per image.
 * Adds image to missing images when not found, for filtering skip.
 */
void resolveImage(Hash selected) {
    std::string imgPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\img";
    for (auto &file : fs::directory_iterator(imgPath)) {
        Hash hash = joaat(fs::path(file).stem().string());
        if (hash != selected) continue;

        std::string fileName = fs::path(file).string();
        auto dims = GetIMGDimensions(fileName);
        unsigned width;
        unsigned height;

        if (dims) {
            width = dims->first;
            height = dims->second;
        }
        else {
            width = 480;
            height = 270;
        }
        int handle = createTexture(fileName.c_str());
        g_addonImages.emplace_back(handle, hash, width, height);
        return;
    }
    g_missingImages.push_back(selected);
}

/*
 * Remove files from the img directory if they aren't present as add-on.
 */
void cleanImageDirectory(bool backup) {
    logger.Write(INFO, "Cleaning img dir");
    std::string imgPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\img";
    std::vector<fs::directory_entry> filesToDiscard;
    for (auto &file : fs::directory_iterator(imgPath)) {
        if (is_directory(fs::path(file))) continue;
        if (fs::path(file).stem().string() == "noimage") continue;
        Hash hash = joaat(fs::path(file).stem().string());
        if (!STREAMING::IS_MODEL_IN_CDIMAGE(hash)) {
            filesToDiscard.push_back(file);
            //logger.Write(INFO, "Marked " + fs::path(file).stem().string());
        }
    }
    std::string bakPath;
    if (filesToDiscard.empty()) {
        logger.Write(INFO, "No files to discard");
        return;
    }
    logger.Write(INFO, "About to discard " + std::to_string(filesToDiscard.size()) + " files");

    if (backup) {
        logger.Write(INFO, "Creating bak dir");
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds >(
            std::chrono::system_clock::now().time_since_epoch()
            ).count();
        bakPath = imgPath + "\\bak." + std::to_string(ms);
        logger.Write(INFO, "Bak dir: " + bakPath);
        fs::create_directory(bakPath);
    }

    for (auto &file : filesToDiscard) {
        std::string src = file.path().string();
        std::wstring srcWide = std::wstring(src.begin(), src.end());
        if (backup) {
            std::string dst = bakPath + "\\" + file.path().filename().string();
            //logger.Write(INFO, "Moving file " + src + " to " + dst);
            std::wstring dstWide = std::wstring(dst.begin(), dst.end());
            MoveFileW(srcWide.c_str(), dstWide.c_str());
        }
        else {
            DeleteFileW(srcWide.c_str());
        }
    }
}

std::string getMakeName(Hash hash) {
    char* makeName = MemoryAccess::GetVehicleMakeName(hash);
    if (makeName == nullptr ||
        strcmp(makeName, "") == 0 ||
        strcmp(HUD::_GET_LABEL_TEXT(makeName), "NULL") == 0) {
        return std::string();
    }
    return std::string(HUD::_GET_LABEL_TEXT(makeName));
}

std::string getModelName(Hash hash) {
    auto modelIt = g_vehicleHashes.find(hash);
    if (modelIt != g_vehicleHashes.end()) return modelIt->second;
    return "NOTFOUND";
}

void cacheDLCVehicles() {
    for (auto &dlc : g_dlcs) {
        dlc.Vehicles.clear();
        for (auto hash : dlc.Hashes) {
            if (!STREAMING::IS_MODEL_IN_CDIMAGE(hash))
                continue;
            char buffer[128];
            sprintf_s(buffer, "VEH_CLASS_%i", VEHICLE::GET_VEHICLE_CLASS_FROM_NAME(hash));
            std::string className = HUD::_GET_LABEL_TEXT(buffer);
            std::string makeName = getMakeName(hash);
            auto notes = std::vector<std::string>{ dlc.Name };
            if (!dlc.Note.empty()) {
                notes.push_back(dlc.Note);
            }
            dlc.Vehicles.emplace_back(className, makeName, getModelName(hash), hash, notes);
            dlc.Classes.emplace(className);
            dlc.Makes.emplace(makeName);
        }
    }
    g_dlcVehicles.clear();
    g_dlcClasses.clear();
    for (const auto& dlc : g_dlcs) {
        for (const ModelInfo& vehicle : dlc.Vehicles) {
            g_dlcVehicles.push_back(vehicle);
        }
        for (const auto& dlcClass : dlc.Classes) {
            g_dlcClasses.emplace(dlcClass);
        }
        for (const auto& dlcMake : dlc.Makes) {
            g_dlcMakes.emplace(dlcMake);
        }
    }
    std::sort(g_dlcVehicles.begin(), g_dlcVehicles.end(), [](const ModelInfo& a1, const ModelInfo& a2) {
        std::string makeName1 = getMakeName(a1.ModelHash);
        std::string makeName2 = getMakeName(a2.ModelHash);

        if (makeName1 == makeName2) {
            std::string modelName1 = getGxtName(a1.ModelHash);
            std::string modelName2 = getGxtName(a2.ModelHash);
            if (modelName1 == modelName2) {
                return getModelName(a1.ModelHash) < getModelName(a2.ModelHash);
            }
            return modelName1 < modelName2;
        }
        return makeName1 < makeName2;
    });
}

void cacheDLCVehicles(std::vector<DLCDefinition>& dlcs) {
    for (auto& dlc : dlcs) {
        dlc.Vehicles.clear();
        for (auto hash : dlc.Hashes) {
            if (!STREAMING::IS_MODEL_IN_CDIMAGE(hash))
                continue;
            char buffer[128];
            sprintf_s(buffer, "VEH_CLASS_%i", VEHICLE::GET_VEHICLE_CLASS_FROM_NAME(hash));
            std::string className = HUD::_GET_LABEL_TEXT(buffer);
            std::string makeName = getMakeName(hash);
            dlc.Vehicles.emplace_back(className, makeName, getModelName(hash), hash);
            dlc.Classes.emplace(className);
            dlc.Makes.emplace(makeName);
        }
    }
}

/*
 * Initialize DLCs and used classes. Categorizes things, though it
 * does feel bad that the specific DLCs aren't retrievable. Adding
 * is easy though, so no harm done.
 */
void cacheDLCs() {
    if (!g_dlcVehicles.empty())
        return;
    cacheDLCVehicles();
}

std::pair<bool, std::vector<std::string>> isHashInDLCList(const std::vector<DLCDefinition>& dlc, Hash hash) {
    auto foundItem = std::find_if(dlc.begin(), dlc.end(), [hash](const DLCDefinition& d) {
        return std::find_if(d.Hashes.begin(), d.Hashes.end(), [hash](const Hash& h) {
            return hash == h;
        }) != d.Hashes.end();
    });

    if (foundItem != dlc.end()) {
        if (!foundItem->Note.empty()) {
            return std::make_pair(true, std::vector<std::string>{ foundItem->Name, foundItem->Note });
        }
        else {
            return std::make_pair(true, std::vector<std::string>{ foundItem->Name });
        }
    }
    return std::make_pair(false, std::vector<std::string>());
}

/**
 * TODO: Rename to "filter" or something
 * Initialize add-ons and used classes. This is ran first so
 * the log outputs a thing.
 */
void cacheAddons() {
    if (!g_addonVehiclesAll.empty() || !g_dlcVehiclesAll.empty())
        return;

    std::vector<Hash> allVehicles;
    for (const auto& hash : g_vehicleHashes) {
        allVehicles.push_back(hash.first);
    }

    std::sort(allVehicles.begin(), allVehicles.end(), [](Hash h1, Hash h2) {
        std::string makeName1 = getMakeName(h1);
        std::string makeName2 = getMakeName(h2);

        if (makeName1 == makeName2) {
            std::string modelName1 = getGxtName(h1);
            std::string modelName2 = getGxtName(h2);
            if (modelName1 == modelName2) {
                return getModelName(h1) < getModelName(h2);
            }
            return modelName1 < modelName2;
        }
        return makeName1 < makeName2;
    });

    int hashLength = 12;
    int nameLength = 20;
    std::stringstream thingy;
    thingy << std::left << std::setw(hashLength) << std::setfill(' ') << "Hash";
    thingy << std::left << std::setw(nameLength) << std::setfill(' ') << "Class";
    thingy << std::left << std::setw(nameLength) << std::setfill(' ') << "Display name";
    thingy << std::left << std::setw(nameLength) << std::setfill(' ') << "Model name";
    thingy << std::left << std::setw(nameLength) << std::setfill(' ') << "GXT name";
    logger.Write(INFO, thingy.str());

    for (auto hash : allVehicles) {
        char buffer[128];
        sprintf_s(buffer, "VEH_CLASS_%i", VEHICLE::GET_VEHICLE_CLASS_FROM_NAME(hash));
        std::string className = HUD::_GET_LABEL_TEXT(buffer);
        std::string displayName = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(hash);
        std::string makeName = getMakeName(hash);

        const auto& [inGameDlc, gameDlcNotes] = isHashInDLCList(g_dlcs, hash);
        if (inGameDlc) {
            g_dlcVehiclesAll.emplace_back(className, makeName, getModelName(hash), hash, gameDlcNotes);
        }
        else {
            std::stringstream hashAsHex;
            std::stringstream logStream;
            hashAsHex << "0x" << std::setfill('0') << std::setw(8) << std::uppercase << std::hex << hash;
            logStream << std::left << std::setw(hashLength) << std::setfill(' ') << hashAsHex.str();
            logStream << std::left << std::setw(nameLength) << std::setfill(' ') << className;
            logStream << std::left << std::setw(nameLength) << std::setfill(' ') << displayName;
            logStream << std::left << std::setw(nameLength) << std::setfill(' ') << getModelName(hash);
            logStream << std::left << std::setw(nameLength) << std::setfill(' ') << getGxtName(hash);
            logger.Write(INFO, logStream.str());
            g_addonVehiclesAll.emplace_back(className, makeName, getModelName(hash), hash);
        }
        const auto& [inUserDlc, userDlcNotes] = isHashInDLCList(g_userDlcs, hash);
        if (!inGameDlc && !inUserDlc) {
            g_addonVehicles.emplace_back(className, makeName, getModelName(hash), hash);
            g_addonClasses.emplace(className);
            g_addonMakes.emplace(makeName);
        }
    }
}

void clearPersistentVehicles() {
    for (Vehicle& veh : g_persistentVehicles) {
        ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, false, true);
        ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&veh);
    }
    g_persistentVehicles.clear();
}

bool findStringInNames(const std::string& search, Hash hash) {
    const char *name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(hash);
    std::string displayName = HUD::_GET_LABEL_TEXT(name);
    std::string rawName = name;
    std::string modelName = getModelName(hash);
    std::string makeNameRaw = MemoryAccess::GetVehicleMakeName(hash);
    std::string makeName = getMakeName(hash);

    if (findSubstring(rawName, search) != -1 ||
        findSubstring(displayName, search) != -1 ||
        findSubstring(modelName, search) != -1 ||
        findSubstring(makeName, search) != -1 ||
        findSubstring(makeNameRaw, search) != -1) {
        return true;
    }
    return false;
}

Vehicle spawnVehicle(Hash hash, Vector3 coords, float heading, DWORD timeout) {
    if (!(STREAMING::IS_MODEL_IN_CDIMAGE(hash) && STREAMING::IS_MODEL_A_VEHICLE(hash))) {
        // Vehicle doesn't exist
        return 0;
    }
    STREAMING::REQUEST_MODEL(hash);
    DWORD startTime = GetTickCount();

    while (!STREAMING::HAS_MODEL_LOADED(hash)) {
        WAIT(0);
        if (GetTickCount() > startTime + timeout) {
            // Couldn't load model
            WAIT(0);
            STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
            return 0;
        }
    }

    Vehicle veh = VEHICLE::CREATE_VEHICLE(hash, coords.x, coords.y, coords.z, heading, 0, 1, 0);
    VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh, 5.0f);
    WAIT(0);
    STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);

    ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, false, true);
    ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&veh);

    return veh;
}

/*
 * Spawns a vehicle with the chosen model hash. Put it on the player when not
 * already in a vehicle, and puts it to the right when a vehicle is already 
 * occupied. Bounding-box dependent, so spawning two jumbojets should have
 * clearance for non-explodiness, and two bikes are spaced without too much
 * distance between 'em.
 */
void spawnVehicle(Hash hash) {
    if (STREAMING::IS_MODEL_IN_CDIMAGE(hash) && STREAMING::IS_MODEL_A_VEHICLE(hash)) {
        Ped playerPed = PLAYER::PLAYER_PED_ID();
        STREAMING::REQUEST_MODEL(hash);
        DWORD startTime = GetTickCount();
        DWORD timeout = 3000; // in millis

        while (!STREAMING::HAS_MODEL_LOADED(hash)) {
            WAIT(0);
            if (GetTickCount() > startTime + timeout) {
                showSubtitle("Couldn't load model");
                WAIT(0);
                STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
                return;
            }
        }

        bool spawnInside = settings.SpawnInside;
        if (findStringInNames("trailer", hash) || findStringInNames("train", hash)) {
            spawnInside = false;
        }

        float offsetX = 0.0f;
        if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, false) || !spawnInside) {
            Vehicle oldVeh = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
            Hash oldHash = ENTITY::GET_ENTITY_MODEL(oldVeh);
            Vector3 newMin, newMax;
            Vector3 oldMin, oldMax;
            MISC::GET_MODEL_DIMENSIONS(hash, &newMin, &newMax);
            MISC::GET_MODEL_DIMENSIONS(oldHash, &oldMin, &oldMax);
            if (!ENTITY::DOES_ENTITY_EXIST(oldVeh)) {
                oldMax.x = oldMin.x = 0.0f;
            }
            // to the right
            // width + margin + width again 
            offsetX = ((newMax.x - newMin.x) / 2.0f) + 1.0f + ((oldMax.x - oldMin.x) / 2.0f);
        }
        
        Vector3 pos = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(playerPed, offsetX, 0.0, 0);

        if (spawnInside && settings.SpawnInplace && PED::IS_PED_IN_ANY_VEHICLE(playerPed, false)) {
            Vehicle oldVeh = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
            Vector3 oldVehiclePos = ENTITY::GET_ENTITY_COORDS(playerPed, true);
            oldVehiclePos = ENTITY::GET_ENTITY_COORDS(oldVeh, true);
            ENTITY::SET_ENTITY_AS_MISSION_ENTITY(oldVeh, true, true);
            VEHICLE::DELETE_VEHICLE(&oldVeh);
            pos = oldVehiclePos;
        }
        

        Vehicle veh = VEHICLE::CREATE_VEHICLE(hash, pos.x, pos.y, pos.z, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()), 0, 1, 0);
        VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh, 5.0f);
        VEHICLE::SET_VEHICLE_DIRT_LEVEL(veh, 0.0f);

        if (spawnInside) {
            ENTITY::SET_ENTITY_HEADING(veh, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()));
            PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, -1);
        }

        WAIT(0);
        STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);

        if (settings.Persistent) {
            ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, true, false);
            g_persistentVehicles.push_back(veh);
        }
        else {
            ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, false, true);
            ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&veh);
        }

        showSubtitle("Spawned " + getGxtName(hash) + " (" + getModelName(hash) + ")");
    }
    else {
        showSubtitle("Vehicle doesn't exist");
    }
}

std::string getImageExtra(Hash addonVehicle) {
    std::string extra;
    AddonImage addonImage;
    if (isHashInImgVector(addonVehicle, g_addonImages, &addonImage)) {
        extra = menu.ImagePrefix + std::to_string(addonImage.TextureID) +
            "W" + std::to_string(addonImage.ResX) +
            "H" + std::to_string(addonImage.ResY);
    }
    else if (std::find(g_missingImages.begin(), g_missingImages.end(), addonVehicle) == g_missingImages.end()) {
        resolveImage(addonVehicle);
    }
    
    if (extra.empty()) {
        extra = menu.ImagePrefix + std::to_string(noImage.TextureID) +
            "W" + std::to_string(noImage.ResX) +
            "H" + std::to_string(noImage.ResY);
    }
    return extra;
}

/*
 * Used by the menu so it gets only the info of the current addon vehicle option,
 * instead of everything. 
 */
std::vector<std::string> resolveVehicleInfo(const ModelInfo& addonVehicle) {
    std::vector<std::string> extras;

    auto modkits = MemoryAccess::GetVehicleModKits(addonVehicle.ModelHash);
    std::string modkitsInfo;
    for (auto kit : modkits) {
        if (kit == modkits.back()) {
            modkitsInfo += std::to_string(kit);
        }
        else {
            modkitsInfo += std::to_string(kit) + ", ";
        }
    }

    extras.push_back(getImageExtra(addonVehicle.ModelHash));

    std::string makeFinal = getMakeName(addonVehicle.ModelHash);
    if (makeFinal.empty())
        makeFinal = "No make";
    extras.push_back("Make: \t" + makeFinal);
    extras.push_back("Name: \t" + getGxtName(addonVehicle.ModelHash));
    extras.push_back("Model: \t" + to_lower(getModelName(addonVehicle.ModelHash)));
    if (!modkitsInfo.empty()) {
        extras.push_back("Mod kit ID(s): \t" + modkitsInfo);
    }
    else {
        extras.emplace_back("Mod kit ID(s): \tNone");
    }
    return extras;
}

/*
 * Since the scripts can be reloaded for dev stuff it'd be nice to just cache
 * the results. InitVehicleArchetype _should_ only be called on game init, so
 * subsequent reloads make for an empty g_vehicleHashes. Cache is updated each
 * full game launch since g_vehicleHashes isn't empty.
 */
void checkCache(const std::string& cacheFile) {
    if (!g_vehicleHashes.empty()) {
        std::ofstream outfile;
        outfile.open(cacheFile, std::ofstream::out | std::ofstream::trunc);
        for (auto hash : g_vehicleHashes) {
            std::string line = std::to_string(hash.first) + " " + hash.second + "\n";
            outfile << line;
        }
    }
    else {
        std::ifstream infile(cacheFile);
        if (infile.is_open()) {
            Hash hash;
            std::string name;
            while (infile >> hash >> name) {
                g_vehicleHashes.insert({ hash, name });
            }
        }
    }
}

void clearAddonLists() {
    g_addonVehiclesAll.clear();
    g_dlcVehiclesAll.clear();
    g_addonVehicles.clear();
    g_addonClasses.clear();
    g_addonMakes.clear();
}

void reloadUserDlc() {
    clearAddonLists();

    g_userDlcs = BuildUserDLCList();
    cacheDLCVehicles(g_userDlcs);

    // Remove user DLC from "normal" add-on pool.
    for (const auto& dlc : g_userDlcs) {
        logger.Write(INFO, "[User] DLC Name: %s", dlc.Name.c_str());
        for (const auto& entry : dlc.Vehicles) {
            logger.Write(INFO, "[User]         : 0x%X / %s", entry.ModelHash, entry.ModelName.c_str());
        }
    }

    // moved after fetching user dlc so it can be excluded in the step
    cacheAddons();
}

void ScriptInit() {
    settingsGeneralFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\settings_general.ini";
    settingsMenuFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\settings_menu.ini";
    settings.SetFiles(settingsGeneralFile);
    settings.ReadSettings();

    menu.RegisterOnMain(std::bind(onMenuOpen));
    menu.RegisterOnExit(std::bind(onMenuExit));
    menu.SetFiles(settingsMenuFile);
    menu.ReadSettings();
    menu.Initialize();

    logger.Write(INFO, "Settings read");

    std::string cacheFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\hashes.cache";
    checkCache(cacheFile); // TODO: This is the prepare section

    MemoryAccess::Init();

    g_dlcs = buildDLClist();
    cacheDLCs();
    reloadUserDlc();

    logger.Write(INFO, "Initialization finished");
}

void InitTextures() {
    clearImages();

    Hash hash = joaat("noimage");
    std::string fileName = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\img\\noimage.png";
    if (FileExists(fileName)) {
        auto dims = GetIMGDimensions(fileName);
        unsigned width;
        unsigned height;
        if (dims) {
            width = dims->first;
            height = dims->second;
        }
        else {
            width = 480;
            height = 270;
        }
        int handle = createTexture(fileName.c_str());
        noImage = AddonImage(handle, hash, width, height);
    }
    else {
        unsigned width = 480;
        unsigned height = 270;
        noImage = AddonImage(-1, hash, width, height);
        logger.Write(ERROR, "Missing img/noimage.png!");
    }

    for (const auto& [modelHash, modelName] : g_vehicleHashes) {
        resolveImage(modelHash);
    }
}

bool initialized = false;

void ScriptTick() {
    while (true) {
        update_menu();
        WAIT(0);
    }
}

void ScriptMain() {
    if (!initialized) {
        logger.Write(INFO, "Script started");
        ScriptInit();
        initialized = true;
    }
    else {
        logger.Write(INFO, "Script restarted");
    }
    InitTextures();
    ScriptTick();
}
