#include "script.h"
#include "keyboard.h"
#include "Util/Util.hpp"

#include "NativeMemory.hpp"

#include "menucontrols.h"
#include "settings.h"
#include "menu.h"
#include "Util/Paths.h"
#include "Util/Logger.hpp"

#include <array>
#include <vector>
#include "VehicleHashes.h"
#include <set>
#include <sstream>
#include <iomanip>
#include "inc/natives.h"

NativeMenu::Menu menu;

std::string settingsGeneralFile;
std::string settingsMenuFile;

Player player;
Ped playerPed;

int prevNotification;

NativeMenu::MenuControls controls;
Settings settings;

// className, vehicleHash
std::vector<std::pair<std::string, Hash>> addonVehicles;
std::set<std::string> addonClasses;

Hash joaat(std::string s) {
	Hash hash = 0;
	for (int i = 0; i < s.size(); i++) {
		hash += s[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash;
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

std::string prettyNameFromHash(Hash hash) {
	char *name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(hash);
	std::string displayName = UI::_GET_LABEL_TEXT(name);
	if (displayName == "NULL") {
		displayName = name;
	}
	return displayName;
}

bool predicateHashByName(Hash h1, Hash h2) {
	std::string name1 = prettyNameFromHash(h1);
	std::string name2 = prettyNameFromHash(h2);
	return name1 < name2;
}

void updateSettings() {
	settings.SaveSettings();
	settings.ReadSettings();
	menu.ReadSettings();
}

void cacheAddons() {
	updateSettings();
	if (!addonVehicles.empty())
		return;
	std::vector<Hash> allVehicles;

	auto vehicleModelList = MemoryAccess::GenerateVehicleModelList();
	int i = 0;
	int g = 0;
	for (auto vehicleModelVector : vehicleModelList) {
		for (auto hash : vehicleModelVector) {
			char *name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(hash);
			
			if (name) {
				allVehicles.push_back(hash);
			}
			i++;
		}
		g++;
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

			addonVehicles.push_back(std::make_pair(className, hash));
			addonClasses.emplace(className);
		}
	}
}

void init() {
	settings.ReadSettings();
	menu.ReadSettings();
	logger.Write("Settings read");
	logger.Write("Initialization finished");
	cacheAddons();
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

void spawnMenu(std::string className) {
	menu.Title(className);
	
	for (auto vehicleHash : addonVehicles) {
		if (className == vehicleHash.first) {
			char *name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(vehicleHash.second);
			std::string displayName = UI::_GET_LABEL_TEXT(name);
			if (displayName == "NULL") {
				displayName = name;
			}
			std::vector<std::string> details = {
				"GXT name: \t" + prettyNameFromHash(vehicleHash.second),
				"Model name: \t" + guessModelName(vehicleHash.second),
			};
			if (menu.Option(displayName, details)) {
				spawnVehicle(vehicleHash.second);
			}
		}
	}
}

std::string manualVehicleName = "";
bool manualSpawnSelected = false;

std::string evaluateInput() {
	PLAYER::IS_PLAYER_CONTROL_ON(false);
	UI::SET_PAUSE_MENU_ACTIVE(false);
	CONTROLS::DISABLE_ALL_CONTROL_ACTIONS(1);
	CONTROLS::IS_CONTROL_ENABLED(playerPed, false);

	for (char c = ' '; c < '~'; c++) {
		int key = str2key(std::string(1, c));
		if (key == -1) continue;
		if (IsKeyJustUp(key)) {
			manualVehicleName += c;
		}
	}
	if (IsKeyJustUp(str2key("SPACE"))) {
		manualVehicleName += ' ';
	}
	if (IsKeyJustUp(str2key("VK_OEM_MINUS"))) {
		manualVehicleName += '_';
	}
	if (IsKeyJustUp(str2key("DELETE")) && manualVehicleName.size() > 0) {
		manualVehicleName.pop_back();
	}
	if (IsKeyJustUp(str2key("BACKSPACE"))) {
		manualVehicleName.clear();
	}

	return manualVehicleName;
}

void clearStuff() {
	manualVehicleName.clear();
}

void update_menu() {
	menu.CheckKeys();

	if (menu.CurrentMenu("mainmenu")) {
		menu.Title("Add-on spawner");

		if (menu.BoolOption("Spawn in car", settings.SpawnInside)) { settings.SaveSettings(); }

		if (settings.SpawnByName) {
			std::vector<std::string> extraSpawnInfo = {
				"Use Delete for backspace",
				"Enter car model:",
				manualVehicleName,
			};

			if (manualSpawnSelected) {
				evaluateInput();
			}

			if (menu.OptionPlus("Spawn by name", extraSpawnInfo, &manualSpawnSelected, nullptr, nullptr, "Enter name")) {
				spawnVehicle(GAMEPLAY::GET_HASH_KEY(CharAdapter(manualVehicleName.c_str())));
			}
		}

		for (auto className : addonClasses) {
			menu.MenuOption(className, className);
		}
	}

	for (auto className : addonClasses) {
		if (menu.CurrentMenu(className)) { spawnMenu(className); }
	}
	menu.EndMenu();
}

void update_game() {
	player = PLAYER::PLAYER_ID();
	playerPed = PLAYER::PLAYER_PED_ID();

	// check if player ped exists and control is on (e.g. not in a cutscene)
	if (!ENTITY::DOES_ENTITY_EXIST(playerPed) || !PLAYER::IS_PLAYER_CONTROL_ON(player)) {
		menu.CloseMenu();
		return;
	}

	// check for player ped death and player arrest
	if (ENTITY::IS_ENTITY_DEAD(playerPed) || PLAYER::IS_PLAYER_BEING_ARRESTED(player, TRUE)) {
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
	logger.Write("Loading " + settingsGeneralFile);
	menu.RegisterOnMain(std::bind(cacheAddons));
	menu.RegisterOnExit(std::bind(clearStuff));
	menu.SetFiles(settingsMenuFile);

	init();
	while (true) {
		update_game();
		WAIT(0);
	}
}

void ScriptMain() {
	srand(GetTickCount());
	main();
}
