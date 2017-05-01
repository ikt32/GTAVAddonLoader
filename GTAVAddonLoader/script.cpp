#include "script.h"
#include "keyboard.h"
#include "Util/Util.hpp"

#include "NativeMemory.hpp"

#include "Menu/controls.h"
#include "settings.h"
#include "Menu/MenuClass.h"
#include "Util/Paths.h"
#include "Util/Logger.hpp"

#include <array>
#include <vector>
#include "VehicleHashes.h"
#include <set>

Menu menu;

std::string settingsGeneralFile;
std::string settingsMenuFile;

Player player;
Ped playerPed;

int prevNotification;

MenuControls controls;
Settings settings;

MemoryAccess mem;
// className, vehicleHash
std::vector<std::pair<std::string, Hash>> addonVehicles;
std::set<std::string> addonClasses;

void cacheAddons() {
	if (!addonVehicles.empty())
		return;
	std::vector<Hash> allVehicles;
	std::vector<Hash> dlcVehicles;
	int num = DLC1::GET_NUM_DLC_VEHICLES();
	for (int i = 0; i < num; i++) {
		dlcVehicles.push_back(DLC1::GET_DLC_VEHICLE_MODEL(i));
	}

	auto vehicleModelList = mem.GenerateVehicleModelList();
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

	logger.Write("Found: ");
	for (auto hash : allVehicles) {
		if (std::find(Vehicles.begin(), Vehicles.end(), hash) == Vehicles.end() &&
			std::find(dlcVehicles.begin(), dlcVehicles.end(), hash) == dlcVehicles.end()) {
			char buffer[128];
			std::sprintf(buffer, "VEH_CLASS_%i", VEHICLE::GET_VEHICLE_CLASS_FROM_NAME(hash));
			char* className = UI::_GET_LABEL_TEXT(buffer);

			char *name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(hash);
			logger.Write(std::string(className) + ": " + std::string(name));
			addonVehicles.push_back(std::make_pair(className, hash));
			addonClasses.emplace(className);
		}
	}
}

void init() {
	settings.ReadSettings(&controls, &menu);
	menu.LoadMenuTheme(std::wstring(settingsMenuFile.begin(), settingsMenuFile.end()).c_str());
	logger.Write("Settings read");
	logger.Write("Initialization finished");
}

void spawnVehicle(Hash hash) {
	if (STREAMING::IS_MODEL_IN_CDIMAGE(hash))
	{
		STREAMING::REQUEST_MODEL(hash);
		while (!STREAMING::HAS_MODEL_LOADED(hash)) WAIT(0);
		Vector3 pos = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 3.0, 2.0, 0);
		Vehicle veh = VEHICLE::CREATE_VEHICLE(hash, pos.x, pos.y, pos.z, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()), 0, 1);
		VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh);
		
		if (settings.SpawnInside) {
			ENTITY::SET_ENTITY_HEADING(veh, ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID()));
			PED::SET_PED_INTO_VEHICLE(PLAYER::PLAYER_PED_ID(), veh, -1);
		}
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(hash);

		showSubtitle("Spawned vehicle");
	}
	else
		showSubtitle("Vehicle doesn't exist");
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
			if (menu.Option(displayName)) {
				spawnVehicle(vehicleHash.second);
			}
		}
	}
}

void update_menu() {
	menu.CheckKeys(&controls, std::bind(cacheAddons), nullptr);

	if (menu.CurrentMenu("mainmenu")) {
		menu.Title("Add-on spawner");

		menu.BoolOption("Spawn in car", &settings.SpawnInside);

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
	settings.SetFiles(settingsGeneralFile, settingsMenuFile);

	logger.Write("Loading " + settingsGeneralFile);
	logger.Write("Loading " + settingsMenuFile);

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
