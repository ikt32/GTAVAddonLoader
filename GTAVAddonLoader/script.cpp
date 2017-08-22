#include "script.h"

#include <array>
#include <experimental/filesystem>
#include <iomanip>
#include <set>
#include <sstream>
#include <vector>
#include <chrono>

#include <inc/natives.h>
#include <menu.h>
#include <menucontrols.h>
#include <NativeMemory.hpp>

#include "Util/Logger.hpp"
#include "Util/Paths.h"
#include "Util/Util.hpp"

#include "keyboard.h"
#include "settings.h"
#include "VehicleHashes.h"
#include "ExtraTypes.h"

namespace fs = std::experimental::filesystem;

NativeMenu::Menu menu;
NativeMenu::MenuControls controls;
Settings settings;

std::string settingsGeneralFile;
std::string settingsMenuFile;

Player player;
Ped playerPed;

int prevNotification;

AddonImage noImage;

std::vector<Hash> GameVehicles;
std::vector<Hash> g_missingImages;
std::vector<AddonVehicle> g_addonVehicles;
std::set<std::string> g_addonClasses;
std::vector<AddonVehicle> g_dlcVehicles;
std::set<std::string> g_dlcClasses;
std::vector<AddonImage> g_addonImages;
std::vector<AddonImageMeta> g_addonImageMetadata;
std::vector<SpriteInfo> g_spriteInfos;
std::vector<DLC> g_dlcs;
std::vector<std::string> g_addonImageNames;

std::vector<Vehicle> g_persistentVehicles;

/*
 * Keep searching for sprites until we're all done. The current value applies for
 * b1103, but if people care to contribute, the texture count for other versions
 * can be added too.
 * Since this function is pretty fast (thanks to Unknown Modder), running it lots
 * shouldn't be problematic.
 * Strangely, it takes multiple tries to get all dictionaries loaded. Calling
 * this a bunch of times when opening DLC-related menus should populate the
 * list fully before the user arrived at something with a picture.
 */
void resolveVehicleSpriteInfo() {
	if (g_spriteInfos.size() == expectedPreviewSprites)
		return;

	for (auto dict : WebsiteDicts) {
		GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT((char*)dict.c_str(), false);
	}
	for (auto dict : WebsiteDicts) {
		GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT((char*)dict.c_str(), false);
		auto textures = MemoryAccess::GetTexturesFromTxd(joaat(dict));
		for (auto texture : textures) {
			SpriteInfo thing;
			if (!isHashInImgVector(joaat(texture->name), g_spriteInfos, &thing)) {
				g_spriteInfos.push_back(SpriteInfo(dict, texture->name, joaat(texture->name), texture->resolutionX, texture->resolutionY));
			}
		}
	}
	logger.Write("Found " + std::to_string(g_spriteInfos.size()) + " preview sprites (dict)");
}

/*
 * Resolving images only when we need it. Should take just 1 tick after an option is selected.
 * Only runs once per image.
 */
void resolveImage(Hash selected) {
	std::string imgPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\img";
	for (auto &file : fs::directory_iterator(imgPath)) {
		Hash hash = joaat(fs::path(file).stem().string());
		if (hash != selected) continue;

		std::string fileName = fs::path(file).string();
		unsigned width;
		unsigned height;
		if (GetIMGDimensions(fileName, &width, &height)) {
			g_addonImageMetadata.push_back(std::make_tuple(fs::path(file).string(), width, height));
		}
		else {
			width = 480;
			height = 270;
		}
		int handle = createTexture(fileName.c_str());
		g_addonImages.push_back(AddonImage(handle, hash, width, height));
		return;
	}
	g_missingImages.push_back(selected);
}

/*
 * Just store all names in the directory so guessModelName has something
 * to work with in cacheAddons.
 */
void storeImageNames() {
	std::string imgPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\img";
	for (auto &file : fs::directory_iterator(imgPath)) {
		g_addonImageNames.push_back(fs::path(file).stem().string());
	}
}

/*
 * Guess model names based on 
 * 1. model name
 * 2. dlcpacks folder name
 * 3. add-on image
 */
std::string guessModelName(Hash hash) {
	// Try display name otherwise (correct vehicles.meta, other package name)
	std::string displayName = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(hash);
	if (joaat(displayName) == hash) return displayName;
	for (int i = 0; i <= 9; i++) {
		if (joaat(displayName + std::to_string(i)) == hash) return displayName + std::to_string(i);
	}
	for (char c = 'a'; c <= 'z'; c++) {
		if (joaat(displayName + c) == hash) return displayName + c;
	}

	// Try dlcpacks dir name (if said DLC only contains the car + variations)
	for (auto folderName : dlcpackFolders) {
		if (joaat(folderName) == hash) return folderName;
		for (int i = 0; i <= 9; i++) {
			if (joaat(folderName + std::to_string(i)) == hash) return folderName + std::to_string(i);
		}
		for (char c = 'a'; c <= 'z'; c++) {
			if (joaat(folderName + c) == hash) return folderName + c;
		}
	}

	// Do we have it stashed from the images?
	for (auto modelName : g_addonImageNames) {
		if (hash == joaat(modelName)) return modelName;
	}

	return "NOTFOUND";
}

// sorting thing
bool predicateHashByName(Hash h1, Hash h2) {
	std::string name1 = prettyNameFromHash(h1);
	std::string name2 = prettyNameFromHash(h2);
	return name1 < name2;
}

// sorting thing 2
bool predicateAddonVehicleHashByName(AddonVehicle a1, AddonVehicle a2) {
	std::string name1 = prettyNameFromHash(a1.second);
	std::string name2 = prettyNameFromHash(a2.second);
	if (name1 == name2) {
		return guessModelName(a1.second) < guessModelName(a2.second);
	}
	return name1 < name2;
}

void cacheDLCVehicles() {
	for (auto &dlc : g_dlcs) {
		dlc.Vehicles.clear();
		for (auto hash : dlc.Hashes) {
			if (!STREAMING::IS_MODEL_IN_CDIMAGE(hash))
				continue;
			char buffer[128];
			sprintf_s(buffer, "VEH_CLASS_%i", VEHICLE::GET_VEHICLE_CLASS_FROM_NAME(hash));
			std::string className = UI::_GET_LABEL_TEXT(buffer);
			dlc.Vehicles.push_back(std::make_pair(className, hash));
			dlc.Classes.emplace(className);
		}
	}
	g_dlcVehicles.clear();
	g_dlcClasses.clear();
	for (auto dlc : g_dlcs) {
		for (AddonVehicle vehicle : dlc.Vehicles) {
			g_dlcVehicles.push_back(vehicle);
		}
		for (auto dlcClass : dlc.Classes) {
			g_dlcClasses.emplace(dlcClass);
		}
	}
	std::sort(g_dlcVehicles.begin(), g_dlcVehicles.end(), predicateAddonVehicleHashByName);
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

/*
 * Initialize add-ons and used classes. This is ran first so
 * the log outputs a thing.
 */
void cacheAddons() {
	if (!g_addonVehicles.empty())
		return;
	std::vector<Hash> allVehicles;

	auto vehicleModelList = MemoryAccess::GenerateVehicleModelList();
	for (auto vehicleModelVector : vehicleModelList) {
		for (auto hash : vehicleModelVector) {
			char *name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(hash);
			
			if (name) {
				allVehicles.push_back(hash);
			}
		}
	}

	std::sort(allVehicles.begin(), allVehicles.end(), predicateHashByName);

	int hashLength = 12;
	int nameLength = 20;
	std::stringstream thingy;
	thingy << std::left << std::setw(hashLength) << std::setfill(' ') << "Hash";
	thingy << std::left << std::setw(nameLength) << std::setfill(' ') << "Class";
	thingy << std::left << std::setw(nameLength) << std::setfill(' ') << "Display name";
	thingy << std::left << std::setw(nameLength) << std::setfill(' ') << "Model name";
	thingy << std::left << std::setw(nameLength) << std::setfill(' ') << "GXT name";
	logger.Write(thingy.str());

	for (auto hash : allVehicles) {
		if (std::find(GameVehicles.begin(), GameVehicles.end(), hash) == GameVehicles.end()) {
			char buffer[128];
			sprintf_s(buffer, "VEH_CLASS_%i", VEHICLE::GET_VEHICLE_CLASS_FROM_NAME(hash));
			std::string className = UI::_GET_LABEL_TEXT(buffer);
			std::string displayName = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(hash);

			std::stringstream hashAsHex;
			std::stringstream logStream;
			hashAsHex << "0x" << std::setfill('0') << std::setw(8) << std::uppercase << std::hex << hash;
			logStream << std::left << std::setw(hashLength) << std::setfill(' ') << hashAsHex.str();
			logStream << std::left << std::setw(nameLength) << std::setfill(' ') << className;
			logStream << std::left << std::setw(nameLength) << std::setfill(' ') << displayName;
			logStream << std::left << std::setw(nameLength) << std::setfill(' ') << guessModelName(hash);
			logStream << std::left << std::setw(nameLength) << std::setfill(' ') << prettyNameFromHash(hash);

			logger.Write(logStream.str());

			g_addonVehicles.push_back(std::make_pair(className, hash));
			g_addonClasses.emplace(className);
		}
	}
}

/*
 * Filter the official DLCs from the list of all vehicles.
 */
void buildBlacklist() {
	GameVehicles.clear();
	for (auto dlc : g_dlcs) {
		for (auto hash : dlc.Hashes) {
			GameVehicles.push_back(hash);
		}
	}
}

void clearPersistentVehicles() {
	for (Vehicle veh : g_persistentVehicles) {
		ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, false, true);
		ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&veh);
	}
	g_persistentVehicles.clear();
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

		float offsetX = 0.0f;
		if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, false) || !settings.SpawnInside) {
			Vehicle oldVeh = PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
			Hash oldHash = ENTITY::GET_ENTITY_MODEL(oldVeh);
			Vector3 newMin, newMax;
			Vector3 oldMin, oldMax;
			GAMEPLAY::GET_MODEL_DIMENSIONS(hash, &newMin, &newMax);
			GAMEPLAY::GET_MODEL_DIMENSIONS(oldHash, &oldMin, &oldMax);
			if (!ENTITY::DOES_ENTITY_EXIST(oldVeh)) {
				oldMax.x = oldMin.x = 0.0f;
			}
			// to the right
			// width + margin + width again 
			offsetX = ((newMax.x - newMin.x) / 2.0f) + 1.0f + ((oldMax.x - oldMin.x) / 2.0f);
		}
		
		Vector3 pos = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), offsetX, 0.0, 0);

		Vehicle veh = VEHICLE::CREATE_VEHICLE(hash, pos.x, pos.y, pos.z, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()), 0, 1);
		VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
		
		if (settings.SpawnInside) {
			ENTITY::SET_ENTITY_HEADING(veh, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()));
			PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, -1);
		}

		WAIT(0);
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);
		//ENTITY::SET_VEHICLE_AS_NO_LONGER_NEEDED(&veh);

		if (settings.Persistent) {
			ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, true, false);
			g_persistentVehicles.push_back(veh);
		}
		else {
			ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, false, true);
			ENTITY::SET_ENTITY_AS_NO_LONGER_NEEDED(&veh);
		}

		showSubtitle("Spawned " + prettyNameFromHash(hash) + " (" + guessModelName(hash) + ")");
	}
	else {
		showSubtitle("Vehicle doesn't exist");
	}
}

/*
 * Used by the menu so it gets only the info of the current addon vehicle option,
 * instead of everything. 
 * Significant performance improvement! 30-ish FPS @ 20 vehicles in class.
 */
std::vector<std::string> resolveVehicleInfo(std::vector<AddonVehicle>::value_type addonVehicle) {
	std::vector<std::string> extras;

	auto modkits = MemoryAccess::GetVehicleModKits(addonVehicle.second);
	std::string modkitsInfo;
	for (auto kit : modkits) {
		if (kit == modkits.back()) {
			modkitsInfo += std::to_string(kit);
		}
		else {
			modkitsInfo += std::to_string(kit) + ", ";
		}
	}

	auto hashIt = std::find(GameVehicles.begin(), GameVehicles.end(), addonVehicle.second);
	AddonImage addonImage;
	SpriteInfo spriteInfo;
	if (isHashInImgVector(addonVehicle.second, g_addonImages, &addonImage)) {
		extras.push_back(menu.ImagePrefix + std::to_string(addonImage.TextureID) +
			"W" + std::to_string(addonImage.ResX) +
			"H" + std::to_string(addonImage.ResY));
	}
	else if (hashIt != GameVehicles.end() && isHashInImgVector(addonVehicle.second, g_spriteInfos, &spriteInfo)) {
		extras.push_back(menu.SpritePrefix +
			spriteInfo.Dict + " " +
			spriteInfo.Name + " " +
			"W" + std::to_string(spriteInfo.ResX) +
			"H" + std::to_string(spriteInfo.ResY));
	}
	else {
		extras.push_back(menu.ImagePrefix + std::to_string(noImage.TextureID) +
			"W" + std::to_string(noImage.ResX) +
			"H" + std::to_string(noImage.ResY));

		if (std::find(g_missingImages.begin(), g_missingImages.end(), addonVehicle.second) == g_missingImages.end()) {
			resolveImage(addonVehicle.second);
		}
	}

	extras.push_back("Model name: \t" + guessModelName(addonVehicle.second));
	if (modkitsInfo.size() > 0) {
		extras.push_back("Mod kit ID(s): \t" + modkitsInfo);
	}
	else {
		extras.push_back("Mod kit ID(s): \tNone");
	}
	return extras;
}

void update_game() {
	player = PLAYER::PLAYER_ID();
	playerPed = PLAYER::PLAYER_PED_ID();

	if (!ENTITY::DOES_ENTITY_EXIST(playerPed)) {
		menu.CloseMenu();
		return;
	}

	update_menu();
}

void main() {
	logger.Write("Script started");

	settingsGeneralFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\settings_general.ini";
	settingsMenuFile = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\settings_menu.ini";
	settings.SetFiles(settingsGeneralFile);
	settings.ReadSettings();

	menu.RegisterOnMain(std::bind(onMenuOpen));
	menu.RegisterOnExit(std::bind(onMenuExit));
	menu.SetFiles(settingsMenuFile);
	menu.ReadSettings();

	logger.Write("Settings read");

	MemoryAccess::initTxdStore();
	g_dlcs = buildDLClist();
	buildBlacklist();
	storeImageNames();
	cacheAddons();
	cacheDLCs();

	Hash hash = joaat("noimage");
	std::string fileName = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\img\\noimage.png";
	if (FileExists(fileName)) {
		unsigned width;
		unsigned height;
		if (!GetIMGDimensions(fileName, &width, &height)) {
			width = 800;
			height = 450;
			logger.Write("Failed to get image proportions for noimage.png, using default values");
		}
		int handle = createTexture(fileName.c_str());
		noImage = AddonImage(handle, hash, width, height);
	}
	else {
		unsigned width = 480;
		unsigned height = 270;
		noImage = AddonImage(-1, hash, width, height);
		logger.Write("Missing img/noimage.png!");
	}
	
	logger.Write("Initialization finished");

	while (true) {
		update_game();
		WAIT(0);
	}
}

void ScriptMain() {
	srand(GetTickCount());
	main();
}
