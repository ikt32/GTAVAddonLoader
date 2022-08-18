#include "script.h"
#include "menu.h"
#include "settings.h"
#include "ExtraTypes.h"
#include "NativeMemory.hpp"
#include "Util/Logger.hpp"
#include "Util/Util.hpp"
#include "Util/Versions.h"

#include <GTAVMenuBase/menukeyboard.h>
#include <inc/natives.h>

#include <set>

std::string manualVehicleName;
std::string searchVehicleName;
bool manualSpawnSelected = false;
bool searchEntrySelected = false;
std::vector<ModelInfo> g_matchedVehicles;

extern NativeMenu::Menu menu;
extern Settings settings;

// Keep a list of vehicles we marked as mission entity
extern std::vector<Vehicle> g_persistentVehicles;

// Stock vehicles DLC. Needs to be updated every DLC release. 
extern std::vector<DLCDefinition> g_dlcs;

// User vehicles DLCs. User-updateable.
extern std::vector<DLCDefinition> g_userDlcs;

// Classes and makes, for grouping in the main menu by either class or make.
extern std::set<std::string> g_addonClasses;   // Grouping-related
extern std::set<std::string> g_addonMakes;     // Grouping-related
extern std::set<std::string> g_dlcClasses;     // Grouping-related
extern std::set<std::string> g_dlcMakes;       // Grouping-related

// These have been filtered by user DLC
extern std::vector<ModelInfo> g_addonVehicles;     // add-on vehicles - used for sorting
extern std::vector<ModelInfo> g_dlcVehicles;       // game vehicles - used for sorting

// These contain everything
extern std::vector<ModelInfo> g_addonVehiclesAll;     // all add-on vehicles - used for sorting
extern std::vector<ModelInfo> g_dlcVehiclesAll;       // all game vehicles - used for sorting

// returns true if a character was pressed
bool evaluateInput(std::string &searchFor) {
    using namespace NativeMenu;
    HUD::SET_PAUSE_MENU_ACTIVE(false);
    PAD::DISABLE_ALL_CONTROL_ACTIONS(2);

    for (char c = ' '; c < '~'; c++) {
        int key = GetKeyFromName(std::string(1, c));
        if (key == -1) continue;
        if (IsKeyJustUp(key)) {
            searchFor += c;
            return true;
        }
    }

    if ((IsKeyDown(GetKeyFromName("LSHIFT")) || IsKeyDown(GetKeyFromName("RSHIFT"))) && IsKeyJustUp(GetKeyFromName("VK_OEM_MINUS"))) {
        searchFor += '_';
        return true;
    }
    if (IsKeyJustUp(GetKeyFromName("VK_OEM_MINUS"))) {
        searchFor += '-';
        return true;
    }
    if (IsKeyJustUp(GetKeyFromName("SPACE"))) {
        searchFor += ' ';
        return true;
    }
    if (IsKeyJustUp(GetKeyFromName("DELETE")) && searchFor.size() > 0) {
        searchFor.pop_back();
        return true;
    }
    if (IsKeyJustUp(GetKeyFromName("BACKSPACE"))) {
        searchFor.clear();
        return true;
    }

    return false;
}

void update_searchresults() {
    g_matchedVehicles.clear();
    for (const auto& addonVehicle : settings.SearchCategory == 0 ? g_dlcVehiclesAll : g_addonVehiclesAll) {
        const char *name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(addonVehicle.ModelHash);
        std::string displayName = HUD::_GET_LABEL_TEXT(name);
        std::string rawName = name;
        std::string modelName = addonVehicle.ModelName;
        std::string makeNameRaw = MemoryAccess::GetVehicleMakeName(addonVehicle.ModelHash);
        std::string makeName = HUD::_GET_LABEL_TEXT(MemoryAccess::GetVehicleMakeName(addonVehicle.ModelHash));

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

void OptionVehicle(const ModelInfo& vehicle) {
    std::string displayName = HUD::_GET_LABEL_TEXT(VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(vehicle.ModelHash));
    std::string displayMakeName = HUD::_GET_LABEL_TEXT(MemoryAccess::GetVehicleMakeName(vehicle.ModelHash));

    if (displayName == "NULL") {
        displayName = vehicle.ModelName;
    }
    std::vector<std::string> extras = {};
    bool visible = false;

    std::string optionText = displayName;
    if (!displayMakeName.empty() && displayMakeName != "NULL") {
        optionText = displayMakeName + " " + displayName;
    }

    if (menu.OptionPlus(optionText, extras, &visible, nullptr, nullptr, "Vehicle info", vehicle.Notes)) {
        spawnVehicle(vehicle.ModelHash);
    }
    if (visible) {
        extras = resolveVehicleInfo(vehicle);
        menu.OptionPlusPlus(extras, "Vehicle info");
    }
}

std::string FormatCategoryName(const std::string& category) {
    if (settings.CategorizeMake &&
        category.empty()) {
        return "No make";
    }
    return category;
}

void update_spawnmenu(const std::string& category, const std::vector<ModelInfo>& addonVehicles, 
                      const std::string& origin, bool asMake) {
    std::string catTitle = category;
    if (asMake && category.empty())
        catTitle = "No make";
    menu.Title(catTitle);
    menu.Subtitle(origin);

    for (const auto& vehicle : addonVehicles) {
        if (category == (asMake ? vehicle.MakeName : vehicle.ClassName)) {
            OptionVehicle(vehicle);
        }
    }
}

void update_mainmenu(const std::set<std::string>& addonCats) {
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
            spawnVehicle(MISC::GET_HASH_KEY((char *)(manualVehicleName.c_str())));
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

    if (!g_userDlcs.empty()) {
        menu.MenuOption("Spawn user DLCs", "userdlcmenu");
    }

    for (const auto& category : addonCats) {
        std::string categoryName = FormatCategoryName(category);
        menu.MenuOption(categoryName, category);
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

    for (const auto& vehicle : g_matchedVehicles) {
        OptionVehicle(vehicle);
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
        clearImages();
    }
    if (menu.Option("Reload user DLC", 
                    { "Reload your custom groupings" })) {
        reloadUserDlc();
    }
    if (menu.Option("Clean up image preview folder", 
                    { "Remove images from the preview folder that aren't detected as add-ons.",
                        "Removed files are put in a \"bak.timestamp\" folder." })) {
        clearImages();
        cleanImageDirectory(true);
    }
    if (settings.Persistent) {
        if (menu.Option("Clear persistence", {"Clears the persistence on spawned vehicles", 
                            "Persistent vehicles: " + std::to_string(g_persistentVehicles.size())})) {
            clearPersistentVehicles();
        }
    }
}

void update_officialdlcmergedmenu(const std::set<std::string>& categories) {
    menu.Title("Official DLC");
    menu.Subtitle("Merged");

    for (const auto& category : categories) {
        std::string categoryName = FormatCategoryName(category);
        menu.MenuOption(categoryName, "dlc_" + category);
    }
}

void update_officialdlcmenu() {
    menu.Title("Official DLC");
    menu.Subtitle("Sort by DLC");

    for (const auto& dlc : g_dlcs) {
        if (!dlc.Note.empty()) {
            menu.MenuOption(dlc.Name, dlc.Name, { dlc.Note });
        }
        else {
            menu.MenuOption(dlc.Name, dlc.Name);
        }
    }
}

void update_userdlcmenu() {
    menu.Title("User DLC");
    menu.Subtitle("User add-on groupings");

    for (const auto& dlc : g_userDlcs) {
        menu.MenuOption(dlc.Name, dlc.Name);
    }
}

void update_perdlcmenu(const DLCDefinition& dlc, const std::set<std::string>& dlcCats) {
    menu.Title(dlc.Name);
    menu.Subtitle("Sort by DLC");

    for (const auto& category : dlcCats) {
        std::string categoryName = FormatCategoryName(category);
        menu.MenuOption(categoryName, dlc.Name + " " + category);
    }
    if (dlcCats.empty()) {
        menu.Option("DLC unavailable.", { "The dlclist.xml and/or game files do not contain the '" + dlc.Name + "' content.",
                        "Game version: " + eGameVersionToString(getGameVersion()) });
    }
}

void update_menu() {
    menu.CheckKeys();
    const std::set<std::string>& addonCats = settings.CategorizeMake ? g_addonMakes : g_addonClasses;

    if (menu.CurrentMenu("mainmenu")) {
        update_mainmenu(addonCats);
    }

    if (menu.CurrentMenu("searchmenu")) {
        update_searchmenu();
    }

    if (menu.CurrentMenu("settingsmenu")) {
        update_settingsmenu();
    }

    for (const auto& category : addonCats) {
        if (menu.CurrentMenu(category)) {
            update_spawnmenu(category, g_addonVehicles, "Add-on vehicles", settings.CategorizeMake);
        }
    }    

    if (settings.MergeDLCs) {
        const std::set<std::string>& categories = settings.CategorizeMake ? g_dlcMakes : g_dlcClasses;

        if (menu.CurrentMenu("officialdlcmergedmenu")) {
            update_officialdlcmergedmenu(categories);
        }
        for (const auto& category : categories) {
            if (menu.CurrentMenu("dlc_" + category)) {
                update_spawnmenu(category, g_dlcVehicles, "Original + All DLCs", settings.CategorizeMake);
            }
        }
    }
    else {
        if (menu.CurrentMenu("officialdlcmenu")) {
            update_officialdlcmenu();
        }

        for (const auto& dlc : g_dlcs) {
            const std::set<std::string>& dlcCats = settings.CategorizeMake ? dlc.Makes : dlc.Classes;

            if (menu.CurrentMenu(dlc.Name)) {
                update_perdlcmenu(dlc, dlcCats);
            }
            for (const auto& className : dlcCats) {
                if (menu.CurrentMenu(dlc.Name + " " + className)) {
                    update_spawnmenu(className, dlc.Vehicles, dlc.Name, settings.CategorizeMake);
                }
            }
        }
    }

    if (menu.CurrentMenu("userdlcmenu")) {
        update_userdlcmenu();
    }

    for (const auto& dlc : g_userDlcs) {
        const std::set<std::string>& dlcCats = settings.CategorizeMake ? dlc.Makes : dlc.Classes;

        if (menu.CurrentMenu(dlc.Name)) {
            update_perdlcmenu(dlc, dlcCats);
        }
        for (const auto& className : dlcCats) {
            if (menu.CurrentMenu(dlc.Name + " " + className)) {
                update_spawnmenu(className, dlc.Vehicles, dlc.Name, settings.CategorizeMake);
            }
        }
    }

    menu.EndMenu();
}
