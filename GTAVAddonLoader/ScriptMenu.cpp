#include "script.h"
#include <set>
#include <inc/natives.h>
#include "Util/Logger.hpp"
#include "Util/Versions.h"
#include "menu.h"
#include "keyboard.h"
#include "settings.h"
#include "ExtraTypes.h"
#include "NativeMemory.hpp"
#include "Util/Util.hpp"

std::string manualVehicleName = "";
std::string searchVehicleName = "";
bool manualSpawnSelected = false;
bool searchEntrySelected = false;
std::vector<ModelInfo> g_matchedVehicles;


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

extern std::vector<Hash> g_gameVehicles;             // all base vehicles
extern std::vector<DLC> g_dlcs;
extern std::set<std::string> g_dlcClasses;
extern std::set<std::string> g_dlcMakes;
extern std::vector<ModelInfo> g_dlcVehicles;
extern std::vector<SpriteInfo> g_dlcSprites;

// returns true if a character was pressed
bool evaluateInput(std::string &searchFor) {
	PLAYER::IS_PLAYER_CONTROL_ON(false);
	UI::SET_PAUSE_MENU_ACTIVE(false);
	CONTROLS::DISABLE_ALL_CONTROL_ACTIONS(2);
	CONTROLS::IS_CONTROL_ENABLED(playerPed, false);

    for (char c = ' '; c < '~'; c++) {
	    int key = str2key(std::string(1, c));
	    if (key == -1) continue;
	    if (IsKeyJustUp(key)) {
		    searchFor += c;
		    return true;
	    }
    }

    if ((IsKeyDown(str2key("LSHIFT")) || IsKeyDown(str2key("RSHIFT"))) && IsKeyJustUp(str2key("VK_OEM_MINUS"))) {
        searchFor += '_';
        return true;
    }
    if (IsKeyJustUp(str2key("VK_OEM_MINUS"))) {
        searchFor += '-';
        return true;
    }
    if (IsKeyJustUp(str2key("SPACE"))) {
        searchFor += ' ';
        return true;
    }
    if (IsKeyJustUp(str2key("DELETE")) && searchFor.size() > 0) {
	    searchFor.pop_back();
	    return true;
    }
    if (IsKeyJustUp(str2key("BACKSPACE"))) {
	    searchFor.clear();
	    return true;
    }

	return false;
}

void update_searchresults() {
    g_matchedVehicles.clear();
    for (auto addonVehicle : settings.SearchCategory == 0 ? g_dlcVehicles : g_addonVehicles) {
        char *name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(addonVehicle.ModelHash);
        std::string displayName = UI::_GET_LABEL_TEXT(name);
        std::string rawName = name;
        if (displayName == "NULL") {
            displayName = name;
        }
        std::string modelName = getModelName(addonVehicle.ModelHash);

        std::string makeNameRaw = MemoryAccess::GetVehicleMakeName(addonVehicle.ModelHash);
        std::string makeName = UI::_GET_LABEL_TEXT(MemoryAccess::GetVehicleMakeName(addonVehicle.ModelHash));

        if (findSubstring(rawName, searchVehicleName) != -1 ||
            findSubstring(displayName, searchVehicleName) != -1 ||
            findSubstring(modelName, searchVehicleName) != -1 ||
            findSubstring(makeName, searchVehicleName) != -1 ||
            findSubstring(makeNameRaw, searchVehicleName) != -1) {
            g_matchedVehicles.push_back(addonVehicle);
        }
    }
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
}

void onMenuExit() {
	manualVehicleName.clear();
}

void format_infobox(std::vector<ModelInfo>::value_type vehicle) {
    char *name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(vehicle.ModelHash);
    std::string displayName = UI::_GET_LABEL_TEXT(name);
    if (displayName == "NULL") {
        displayName = name;
    }
    std::vector<std::string> extras = {};
    bool visible = false;
    if (menu.OptionPlus(displayName, extras, &visible, nullptr, nullptr, "Vehicle info", {})) {
        spawnVehicle(vehicle.ModelHash);
    }
    if (visible) {
        extras = resolveVehicleInfo(vehicle);
        menu.OptionPlusPlus(extras, "Vehicle info");
    }
}

void update_spawnmenu(std::string category, std::vector<ModelInfo> addonVehicles, std::string origin, bool asMake) {
	menu.Title(category);
	menu.Subtitle(origin);

	for (auto vehicle : addonVehicles) {
		if (category == (asMake ? vehicle.MakeName : vehicle.ClassName)) {
            format_infobox(vehicle);
		}
	}
}

void update_mainmenu(std::set<std::string> addonCats) {
	menu.Title("Add-on spawner");
	menu.Subtitle("~b~" + std::string(DISPLAY_VERSION) + "~w~");

	menu.MenuOption("Settings", "settingsmenu");

	if (settings.SearchMenu) {
		if (menu.MenuOption("Search vehicles", "searchmenu")) {
            update_searchresults();
		}
	}

	if (settings.SpawnByName) {
		std::vector<std::string> extraSpawnInfo = {
			"Use Delete for backspace",
			"Enter car model:",
			manualVehicleName,
		};

		if (manualSpawnSelected) {
			evaluateInput(manualVehicleName);
		}

		if (menu.OptionPlus("Spawn by name", extraSpawnInfo, &manualSpawnSelected, nullptr, nullptr, "Enter name")) {
			spawnVehicle(GAMEPLAY::GET_HASH_KEY((char *)(manualVehicleName.c_str())));
		}
	}

	if (settings.ListAllDLCs) {
		if (settings.MergeDLCs) {
            menu.MenuOption("Spawn official DLCs", "officialdlcmergedmenu");
		}
		else {
            menu.MenuOption("Spawn official DLCs", "officialdlcmenu");
		}
	}

	for (auto category : addonCats) {
		menu.MenuOption(category, category);
	}
}

void update_searchmenu() {
	menu.Title("Search");
	menu.Subtitle("");

	std::vector<std::string> extraSpawnInfo = {
		"Use Delete for backspace",
		"Searching for:",
		searchVehicleName,
	};

	if (searchEntrySelected) {
		if (evaluateInput(searchVehicleName)) {
			update_searchresults();
		}
	}

	if (menu.StringArray("Search in", { "Game vehicles", "Add-on vehicles" }, settings.SearchCategory)) {
		update_searchresults();
	}

	if (menu.OptionPlus("Search for ...", extraSpawnInfo, &searchEntrySelected, nullptr, nullptr, "Search entry")) {
		update_searchresults();
	}

	for (auto vehicle : g_matchedVehicles) {
		format_infobox(vehicle);
	}
}

void update_settingsmenu() {
	menu.Title("Settings");
	menu.Subtitle("");

	if (menu.BoolOption("Spawn inside vehicle", settings.SpawnInside)) {
		settings.SaveSettings();
	}
	if (menu.BoolOption("Spawn in place", settings.SpawnInplace, 
	                    { "Don't spawn to the right of the previous car, but spawn at the current position. This replaces the current vehicle.",
		                    "Only active if \"Spawn inside vehicle\" is turned on."})) {
		settings.SaveSettings();
	}
	if (menu.BoolOption("Enable persistence", settings.Persistent,
						{ "Spawned cars don't disappear." })) {
		settings.SaveSettings();
	}
	if (menu.BoolOption("Spawn by name", settings.SpawnByName,
						{ "Spawn vehicles by their model name.",
							"This setting adds an option to the main menu." })) {
		settings.SaveSettings();
	}
	if (menu.BoolOption("Categorize by make", settings.CategorizeMake, 
	                    { "Categorizing by " + std::string(settings.CategorizeMake ? "make" : "class") + "." })) {
		settings.SaveSettings();
	}
	if (menu.BoolOption("List all DLCs", settings.ListAllDLCs,
	                    { "Show all official DLC vehicles. These will appear in their own submenu, sorted per class, per DLC." })) {
		settings.SaveSettings();
	}
	if (menu.BoolOption("Merge DLCs", settings.MergeDLCs,
	                    { "Don't sort per DLC and just show the vehicles per class." })) {
		settings.SaveSettings();
	}
	if (menu.BoolOption("Enable search menu", settings.SearchMenu, 
						{ "Search for vehicles by their make, game name or model name.",
							"This setting adds an option to the main menu." })) {
		settings.SaveSettings();
	}
	if (menu.Option("Reload previews", 
					{ "Use for when you changed an image that's already been loaded."})) {
		resolveVehicleSpriteInfo();

		g_missingImages.clear();
		g_addonImages.clear();
	}
	if (menu.Option("Clean up image preview folder", 
					{ "Remove images from the preview folder that aren't detected as add-ons.",
						"Removed files are put in a \"bak.timestamp\" folder." })) {
		g_missingImages.clear();
		g_addonImages.clear();

		cleanImageDirectory(true);
	}

	if (settings.Persistent) {
		if (menu.Option("Clear persistence", {"Clears the persistence on spawned vehicles", 
			                "Persistent vehicles: " + std::to_string(g_persistentVehicles.size())})) {
			clearPersistentVehicles();
		}
	}
}

void update_officialdlcmergedmenu(std::set<std::string> categories) {
	menu.Title("Official DLC");
	menu.Subtitle("Merged");

	for (auto category : categories) {
		menu.MenuOption(category, "dlc_" + category);
	}
}

void update_officialdlcmenu() {
	menu.Title("Official DLC");
	menu.Subtitle("Sort by DLC");

	for (auto dlc : g_dlcs) {
        menu.MenuOption(dlc.Name, dlc.Name);
	}
}

void update_perdlcmenu(std::vector<DLC>::value_type dlc, std::set<std::string> dlcCats) {
	menu.Title(dlc.Name);
	menu.Subtitle("Sort by DLC");

	for (auto category : dlcCats) {
        menu.MenuOption(category, dlc.Name + " " + category);
	}
	if (dlcCats.empty()) {
		menu.Option("DLC unavailable.", { "This version of the game does not have the " + dlc.Name + " DLC content.",
			            "Game version: " + eGameVersionToString(getGameVersion()) });
	}
}

void update_menu() {
	menu.CheckKeys();
	std::set<std::string> &addonCats = settings.CategorizeMake ? g_addonMakes : g_addonClasses;

	if (menu.CurrentMenu("mainmenu")) {
		update_mainmenu(addonCats);
	}

	if (menu.CurrentMenu("searchmenu")) {
		update_searchmenu();
	}

	if (menu.CurrentMenu("settingsmenu")) {
		update_settingsmenu();
	}

	for (auto category : addonCats) {
		if (menu.CurrentMenu(category)) {
			update_spawnmenu(category, g_addonVehicles, "Add-on vehicles", settings.CategorizeMake);
		}
	}	

	if (settings.MergeDLCs) {
		std::set<std::string> &categories = settings.CategorizeMake ? g_dlcMakes : g_dlcClasses;

		if (menu.CurrentMenu("officialdlcmergedmenu")) {
			update_officialdlcmergedmenu(categories);
		}
		for (auto category : categories) {
			if (menu.CurrentMenu("dlc_" + category)) {
				update_spawnmenu(category, g_dlcVehicles, "Original + All DLCs", settings.CategorizeMake);
			}
		}
	}
	else {
		if (menu.CurrentMenu("officialdlcmenu")) {
			update_officialdlcmenu();
		}

		for (auto dlc : g_dlcs) {
			std::set<std::string> &dlcCats = settings.CategorizeMake ? dlc.Makes : dlc.Classes;

			if (menu.CurrentMenu(dlc.Name)) {
				update_perdlcmenu(dlc, dlcCats);
			}
			for (auto className : dlcCats) {
				if (menu.CurrentMenu(dlc.Name + " " + className)) {
					update_spawnmenu(className, dlc.Vehicles, dlc.Name, settings.CategorizeMake);
				}
			}
		}
	}

	menu.EndMenu();
}
