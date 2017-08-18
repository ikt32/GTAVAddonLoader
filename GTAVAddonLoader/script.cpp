#include "script.h"

#include <array>
#include <experimental/filesystem>
#include <iomanip>
#include <set>
#include <sstream>
#include <vector>

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
int noImageHandle;

std::vector<Hash> Vehicles;
std::vector<AddonVehicle> g_addonVehicles;
std::set<std::string> g_addonClasses;
std::vector<AddonVehicle> g_dlcVehicles;
std::set<std::string> g_dlcClasses;
std::vector<AddonImage> g_addonImages;
std::vector<AddonImageMeta> g_addonImageMetadata;
std::vector<SpriteInfo> g_spriteInfos;
std::vector<DLC> g_dlcs;

void resolveVehicleSpriteInfo() {
	g_spriteInfos.clear();
	for (auto dict : WebsiteDicts) {
		if (!GRAPHICS::HAS_STREAMED_TEXTURE_DICT_LOADED((char*)dict.c_str())) {
			GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT((char*)dict.c_str(), false);
		}
		auto textures = MemoryAccess::GetTexturesFromTxd(joaat(dict));
		for (auto texture : textures) {
			g_spriteInfos.push_back(SpriteInfo(dict, texture->name, joaat(texture->name), texture->resolutionX, texture->resolutionY));
		}
	}
}

// Uses file operations
void resolveImgs() {
	g_addonImages.clear();
	g_addonImageMetadata.clear();
	std::string imgPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\img";
	for (auto &file : fs::directory_iterator(imgPath)) {
		std::stringstream fileName;
		unsigned width;
		unsigned height;
		fileName << file;
		if (GetIMGDimensions(fileName.str(), &width, &height)) {
			g_addonImageMetadata.push_back(std::make_tuple(fileName.str(), width, height));
		}
	}
	for (auto metadata : g_addonImageMetadata) {
		auto fileName = std::get<0>(metadata);
		Hash hash = joaat(fs::path(fileName).stem().string());
		AddonImage thing;
		if (isHashInImgVector(hash, g_addonImages, &thing)) {
			continue;
		}
		auto width = std::get<1>(metadata);
		auto height = std::get<2>(metadata);
		int handle = createTexture(fileName.c_str());
		if (hash == joaat("noimage"))
			noImageHandle = handle;
		g_addonImages.push_back(AddonImage(handle, hash, width, height));
	}
	logger.Write("Found " + std::to_string(g_addonImages.size()) + " preview images.");
}

std::string guessModelName(Hash hash) {
	// Try dlcpacks dir name (if said DLC only contains the car + variations)
	for (auto folderName : dlcpackFolders) {
		std::transform(folderName.begin(), folderName.end(), folderName.begin(), ::tolower);
		if (joaat(folderName) == hash) return folderName;
		for (int i = 0; i <= 9; i++) {
			if (joaat(folderName + std::to_string(i)) == hash) return folderName + std::to_string(i);
		}
		for (char c = 'a'; c <= 'z'; c++) {
			if (joaat(folderName + c) == hash) return folderName + c;
		}
	}

	// Do we have it stashed from the images?
	for (auto metadata : g_addonImageMetadata) {
		auto fileName = std::get<0>(metadata);
		std::string modelName = fs::path(fileName).stem().string();
		if (hash == joaat(modelName)) return modelName;
	}

	// Try display name otherwise (correct vehicles.meta, other package name)
	std::string displayName = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(hash);
	std::transform(displayName.begin(), displayName.end(), displayName.begin(), ::tolower);
	if (joaat(displayName) == hash) return displayName;
	for (int i = 0; i <= 9; i++) {
		if (joaat(displayName + std::to_string(i)) == hash) return displayName + std::to_string(i);
	}
	for (char c = 'a'; c <= 'z'; c++) {
		if (joaat(displayName + c) == hash) return displayName + c;
	}
	return "NOTFOUND";
}

bool predicateHashByName(Hash h1, Hash h2) {
	std::string name1 = prettyNameFromHash(h1);
	std::string name2 = prettyNameFromHash(h2);
	return name1 < name2;
}

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

void buildDLClist() {
	g_dlcs = {
		{ DLC("Original game", OriginalVehicles) },
		{ DLC("Returning Player", SPUpgradeVehicles) } ,
		{ DLC("Beach Bum", BeachBumVehicles) },
		{ DLC("Valentines Day Massacre", ValentineVehicles) },
		{ DLC("The Business Update", BusinessVehicles) },
		{ DLC("The High Life", HighLifeVehicles) },
		{ DLC("I'm Not A Hipster", HipsterVehicles) },
		{ DLC("Independence Day", MURKAVehicles) },
		{ DLC("SA Flight School", FlightSchoolVehicles) },
		{ DLC("Last Team Standing", LTSVehicles) },
		{ DLC("Festive Surprise", FestiveVehicles) },
		{ DLC("Heists", HeistsVehicles) },
		{ DLC("Ill-Gotten Gains Pt1", IllGottenGainsPt1Vehicles) },
		{ DLC("Ill-Gotten Gains Pt2", IllGottenGainsPt2Vehicles) },
		{ DLC("Lowriders", LowriderVehicles) },
		{ DLC("Halloween Surprise", HalloweenVehicles) },
		{ DLC("Executives and Other Criminals", ExecutiveVehicles) },
		{ DLC("Drop Zone", DropzoneVehicles) },
		{ DLC("Lowriders: Custom Classics", LowriderCCVehicles) },
		{ DLC("Further Adventures in Finance and Felony", FinanceFelonyVehicles) },
		{ DLC("Cunning Stunts", CunningStuntsVehicles) },
		{ DLC("Bikers", BikersVehicles) },
		{ DLC("Import/Export", ImportExportVehicles) },
		{ DLC("Cunning Stunts: Special Vehicle Circuit", CunningStunts2Vehicles) },
		{ DLC("Gunrunning", GunrunningVehicles)}
	};
}

void cacheDLCs() {
	buildDLClist();
	cacheDLCVehicles();
}

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
		if (std::find(Vehicles.begin(), Vehicles.end(), hash) == Vehicles.end()) {
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

void buildBlacklist() {
	Vehicles.clear();
	for (auto dlc : g_dlcs) {
		for (auto hash : dlc.Hashes) {
			Vehicles.push_back(hash);
		}
	}
}

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
		ENTITY::SET_VEHICLE_AS_NO_LONGER_NEEDED(&veh);

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

	auto hashIt = std::find(Vehicles.begin(), Vehicles.end(), addonVehicle.second);
	AddonImage addonImage;
	SpriteInfo spriteInfo;
	if (isHashInImgVector(addonVehicle.second, g_addonImages, &addonImage)) {
		extras.push_back(menu.ImagePrefix + std::to_string(addonImage.TextureID) +
			"W" + std::to_string(addonImage.ResX) +
			"H" + std::to_string(addonImage.ResY));
	}
	else if (hashIt != Vehicles.end() && isHashInImgVector(addonVehicle.second, g_spriteInfos, &spriteInfo)) {
		extras.push_back(menu.SpritePrefix +
			spriteInfo.Dict + " " +
			spriteInfo.Name + " " +
			"W" + std::to_string(spriteInfo.ResX) +
			"H" + std::to_string(spriteInfo.ResY));
	}
	else {
		extras.push_back(menu.ImagePrefix + std::to_string(noImageHandle) +
			"W" + std::to_string(320) +
			"H" + std::to_string(180));
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

	// check if player ped exists and control is on (e.g. not in a cutscene)
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
	buildDLClist();
	buildBlacklist();
	cacheAddons();
	cacheDLCs();

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
