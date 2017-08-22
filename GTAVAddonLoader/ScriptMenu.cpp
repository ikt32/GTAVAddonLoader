#include "script.h"
#include <set>
#include <inc/natives.h>
#include "Util/Logger.hpp"
#include "Util/Versions.h"
#include "menu.h"
#include "keyboard.h"
#include "settings.h"
#include "ExtraTypes.h"

std::string manualVehicleName = "";
bool manualSpawnSelected = false;

extern NativeMenu::Menu menu;
extern NativeMenu::MenuControls controls;
extern Settings settings;
extern Ped playerPed;

extern std::vector<Hash> g_missingImages;
extern std::vector<Vehicle> g_persistentVehicles;

extern std::set<std::string> g_addonClasses;
extern std::set<std::string> g_addonMakes;
extern std::vector<ModelInfo> g_addonVehicles;	// all add-on vehicles
extern std::vector<AddonImage> g_addonImages;
extern std::vector<AddonImageMeta> g_addonImageMetadata;
extern std::vector<std::string> g_addonImageNames; // just all filenames separately for hashing in begin

extern std::vector<Hash> GameVehicles;             // all base vehicles
extern std::vector<DLC> g_dlcs;
extern std::set<std::string> g_dlcClasses;
extern std::set<std::string> g_dlcMakes;
extern std::vector<ModelInfo> g_dlcVehicles;
extern std::vector<SpriteInfo> g_dlcSprites;

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

void updateSettings() {
	settings.SaveSettings();
	settings.ReadSettings();
	menu.ReadSettings();
}

void onMenuOpen() {
	updateSettings();
	cacheAddons();
	cacheDLCs();
	resolveVehicleSpriteInfo();
}

void onMenuExit() {
	manualVehicleName.clear();
}

void spawnMenu(std::string className, std::vector<ModelInfo> addonVehicles, std::string origin) {
	menu.Title(className);
	menu.Subtitle(origin);
	for (auto addonVehicle : addonVehicles) {
		if (className == addonVehicle.ClassName) {
			char *name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(addonVehicle.ModelHash);
			std::string displayName = UI::_GET_LABEL_TEXT(name);
			if (displayName == "NULL") {
				displayName = name;
			}

			std::vector<std::string> extras = {};
			bool visible = false;
			if (menu.OptionPlus(displayName, extras, &visible, nullptr, nullptr, "Add-on info", {})) {
				spawnVehicle(addonVehicle.ModelHash);
			}
			if (visible) {
				extras = resolveVehicleInfo(addonVehicle);
				menu.OptionPlusPlus(extras, "Add-on info");
			}
		}
	}
}

void update_menu() {
	menu.CheckKeys();

	if (menu.CurrentMenu("mainmenu")) {
		menu.Title("Add-on spawner");
		menu.Subtitle(DISPLAY_VERSION, false);

		menu.MenuOption("Settings", "settingsmenu");

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
				spawnVehicle(GAMEPLAY::GET_HASH_KEY((char *)(manualVehicleName.c_str())));
			}
		}

		if (settings.ListAllDLCs) {
			if (settings.MergeDLCs) {
				if (menu.MenuOption("Spawn official DLCs", "officialdlcmergedmenu")) {
					resolveVehicleSpriteInfo();
				}
			}
			else {
				if (menu.MenuOption("Spawn official DLCs", "officialdlcmenu")) {
					resolveVehicleSpriteInfo();
				}
			}
		}

		for (auto className : g_addonClasses) {
			menu.MenuOption(className, className);
		}
	}

	if (menu.CurrentMenu("settingsmenu")) {
		menu.Title("Settings");
		menu.Subtitle("Add-on spawner settings");

		if (menu.BoolOption("Spawn in car", settings.SpawnInside)) {
			settings.SaveSettings();
		}
		if (menu.BoolOption("Spawned cars are persistent", settings.Persistent)) {
			settings.SaveSettings();
		}
		if (menu.BoolOption("Spawn manually", settings.SpawnByName)) {
			settings.SaveSettings();
		}
		if (menu.BoolOption("List all DLCs", settings.ListAllDLCs,
		{ "Show all official DLC vehicles."
			" These will appear in their own submenu, sorted per class, per DLC." })) {
			settings.SaveSettings();
		}
		if (menu.BoolOption("Merge DLCs", settings.MergeDLCs,
		{ "Don't sort per DLC and just show the vehicles per class." })) {
			settings.SaveSettings();
		}
		if (menu.Option("Reload previews", { "Use for when you changed an image "
			"that's already been loaded."})) {
			resolveVehicleSpriteInfo();

			g_addonImageNames.clear();
			g_missingImages.clear();
			g_addonImages.clear();
			g_addonImageMetadata.clear();
			storeImageNames();
		}

		if (settings.Persistent) {
			if (menu.Option("Clear persistence", {"Clears the persistence on spawned vehicles", 
				"Persistent vehicles: " + std::to_string(g_persistentVehicles.size())})) {
				clearPersistentVehicles();
			}
		}
	}

	for (auto className : g_addonClasses) {
		if (menu.CurrentMenu(className)) { spawnMenu(className, g_addonVehicles, "Add-on vehicles"); }
	}

	if (!settings.MergeDLCs) {
		if (menu.CurrentMenu("officialdlcmenu")) {
			menu.Title("Official DLC");
			menu.Subtitle("");

			for (auto dlc : g_dlcs) {
				if (menu.MenuOption(dlc.Name, dlc.Name)) {
					resolveVehicleSpriteInfo();
				}
			}
		}

		for (auto dlc : g_dlcs) {
			if (menu.CurrentMenu(dlc.Name)) {
				menu.Title(dlc.Name);
				menu.Subtitle("By DLC");

				for (auto className : dlc.Classes) {
					if (menu.MenuOption(className, dlc.Name + " " + className)) {
						resolveVehicleSpriteInfo();
					}
				}
				if (dlc.Classes.empty()) {
					menu.Option("DLC unavailable.", { "This version of the game does not have the " + dlc.Name + " DLC content.",
						"Game version: " + eGameVersionToString(getGameVersion()) });
				}
			}
			for (auto className : dlc.Classes) {
				if (menu.CurrentMenu(dlc.Name + " " + className)) {
					menu.Title(className);

					spawnMenu(className, dlc.Vehicles, dlc.Name);
				}
			}
		}
	}
	else {
		if (menu.CurrentMenu("officialdlcmergedmenu")) {
			menu.Title("Official DLC");
			menu.Subtitle("Merged");

			for (auto className : g_dlcClasses) {
				menu.MenuOption(className, "dlc_" + className);
			}
		}
		for (auto className : g_dlcClasses) {
			if (menu.CurrentMenu("dlc_" + className)) { spawnMenu(className, g_dlcVehicles, "Original + All DLCs"); }
		}
	}


	menu.EndMenu();
}
