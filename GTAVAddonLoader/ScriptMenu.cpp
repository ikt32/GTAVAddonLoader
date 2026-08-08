#include "script.h"
#include "menu.h"
#include "settings.h"
#include "GitInfo.h"
#include "ExtraTypes.h"
#include "Language.h"
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

namespace {
std::string T(const char* key) {
    return gLanguage.Text(key);
}
}

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
bool evaluateInput(std::string& searchFor) {
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
        const char* name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(addonVehicle.ModelHash);
        std::string displayName = HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION(name);
        std::string rawName = name;
        std::string modelName = addonVehicle.ModelName;
        std::string makeNameRaw = MemoryAccess::GetVehicleMakeName(addonVehicle.ModelHash);
        std::string makeName = HUD::GET_FILENAME_FOR_AUDIO_CONVERSATION(makeNameRaw.c_str());

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
    reloadLanguages();
    cacheAddons();
    cacheDLCs();
}

void onMenuExit() {
    manualVehicleName.clear();
}

void OptionVehicle(const ModelInfo& vehicle) {
    std::string displayName = Utility::GetVehicleNameGxt(vehicle.ModelHash);
    std::string displayMakeName = Utility::GetVehicleMakeGxt(vehicle.ModelHash);

    if (displayName.empty() || displayName == "NULL") {
        displayName = vehicle.ModelName;
    }
    std::vector<std::string> extras = {};
    bool visible = false;

    std::string optionText = displayName;
    if (!displayMakeName.empty() && displayMakeName != "NULL") {
        optionText = displayMakeName + " " + displayName;
    }
    if (menu.OptionPlus(optionText, extras, &visible, nullptr, nullptr, T("vehicle.info.title"), vehicle.Notes)) {
        spawnVehicle(vehicle.ModelHash);
    }
    if (visible) {
        extras = resolveVehicleInfo(vehicle);
        menu.OptionPlusPlus(extras, T("vehicle.info.title"));
    }
}

std::string FormatCategoryName(const std::string& category) {
    if (settings.CategorizeMake &&
        category.empty()) {
        return T("common.no_make");
    }
    return category;
}

void update_spawnmenu(const std::string& category, const std::vector<ModelInfo>& addonVehicles,
                      const std::string& origin, bool asMake) {
    std::string catTitle = category;
    if (asMake && category.empty())
        catTitle = T("common.no_make");
    menu.Title(catTitle);
    menu.Subtitle(origin);

    for (const auto& vehicle : addonVehicles) {
        if (category == (asMake ? vehicle.MakeName : vehicle.ClassName)) {
            OptionVehicle(vehicle);
        }
    }
}

void update_mainmenu(const std::set<std::string>& addonCats) {
    menu.Title(T("menu.main.title"));
    menu.Subtitle(std::format("~b~{}{}~w~", DISPLAY_VERSION, GIT_DIFF));

    menu.MenuOption(T("menu.main.settings"), "settingsmenu");

    if (settings.SearchMenu) {
        if (menu.MenuOption(T("menu.main.search_vehicles"), "searchmenu")) {
            update_searchresults();
        }
    }

    if (settings.SpawnByName) {
        std::vector<std::string> extraSpawnInfo = {
            T("input.delete_backspace"),
            T("input.enter_model"),
            manualVehicleName,
        };

        if (manualSpawnSelected) {
            evaluateInput(manualVehicleName);
        }

        if (menu.OptionPlus(T("menu.main.spawn_by_name"), extraSpawnInfo, &manualSpawnSelected, nullptr, nullptr, T("input.enter_name"))) {
            spawnVehicle(MISC::GET_HASH_KEY((char*)(manualVehicleName.c_str())));
        }
    }

    if (settings.ListAllDLCs) {
        if (settings.MergeDLCs) {
            menu.MenuOption(T("menu.main.spawn_official_dlcs"), "officialdlcmergedmenu");
        }
        else {
            menu.MenuOption(T("menu.main.spawn_official_dlcs"), "officialdlcmenu");
        }
    }

    if (!g_userDlcs.empty()) {
        menu.MenuOption(T("menu.main.spawn_user_dlcs"), "userdlcmenu");
    }

    for (const auto& category : addonCats) {
        std::string categoryName = FormatCategoryName(category);
        menu.MenuOption(categoryName, category);
    }
}

void update_searchmenu() {
    menu.Title(T("menu.search.title"));
    menu.Subtitle("");

    std::vector<std::string> extraSpawnInfo = {
        T("input.delete_backspace"),
        T("menu.search.searching_for"),
        searchVehicleName,
    };

    if (searchEntrySelected) {
        if (evaluateInput(searchVehicleName)) {
            update_searchresults();
        }
    }

    if (menu.StringArray(T("menu.search.scope"), { T("menu.search.scope.game"), T("menu.search.scope.addon") }, settings.SearchCategory)) {
        update_searchresults();
    }

    if (menu.OptionPlus(T("menu.search.action"), extraSpawnInfo, &searchEntrySelected, nullptr, nullptr, T("menu.search.entry"))) {
        update_searchresults();
    }

    for (const auto& vehicle : g_matchedVehicles) {
        OptionVehicle(vehicle);
    }
}

void update_settingsmenu() {
    menu.Title(T("menu.settings.title"));
    menu.Subtitle("");

    const auto& languages = gLanguage.Languages();
    std::vector<std::string> languageNames;
    int languageIndex = 0;
    for (size_t index = 0; index < languages.size(); ++index) {
        languageNames.push_back(languages[index].Name);
        if (languages[index].Code == gLanguage.ActiveCode())
            languageIndex = static_cast<int>(index);
    }
    if (!languageNames.empty() && menu.StringArray(T("menu.settings.language"), languageNames, languageIndex)) {
        if (languageIndex >= 0 && languageIndex < static_cast<int>(languages.size()) &&
            gLanguage.Select(languages[languageIndex].Code)) {
            settings.Language = gLanguage.ActiveCode();
            settings.SaveSettings();
        }
    }

    if (menu.BoolOption(T("settings.spawn_inside"), settings.SpawnInside)) {
        settings.SaveSettings();
    }
    if (menu.BoolOption(T("settings.spawn_in_place"), settings.SpawnInplace,
                        { T("settings.spawn_in_place.detail1"), T("settings.spawn_in_place.detail2") })) {
        settings.SaveSettings();
    }
    if (menu.BoolOption(T("settings.persistence"), settings.Persistent,
                        { T("settings.persistence.detail") })) {
        settings.SaveSettings();
    }
    if (menu.BoolOption(T("settings.spawn_max_perf_mods"), settings.SpawnMaxPerfMods,
                        { T("settings.spawn_max_perf_mods.detail") })) {
        settings.SaveSettings();
    }
    if (menu.BoolOption(T("settings.spawn_by_name"), settings.SpawnByName,
                        { T("settings.spawn_by_name.detail1"), T("settings.adds_main_option") })) {
        settings.SaveSettings();
    }
    if (menu.BoolOption(T("settings.categorize_make"), settings.CategorizeMake,
                        { gLanguage.Format("settings.categorize_make.detail", {
                            { "category", settings.CategorizeMake ? T("settings.category.make") : T("settings.category.class") }
                        }) })) {
        settings.SaveSettings();
    }
    if (menu.BoolOption(T("settings.list_all_dlcs"), settings.ListAllDLCs,
                        { T("settings.list_all_dlcs.detail") })) {
        settings.SaveSettings();
    }
    if (menu.BoolOption(T("settings.merge_dlcs"), settings.MergeDLCs,
                        { T("settings.merge_dlcs.detail") })) {
        settings.SaveSettings();
    }
    if (menu.BoolOption(T("settings.search_menu"), settings.SearchMenu,
                        { T("settings.search_menu.detail"), T("settings.adds_main_option") })) {
        settings.SaveSettings();
    }

    if (menu.BoolOption(T("settings.show_previews"), settings.ShowPreviews,
                        { T("settings.show_previews.detail") })) {
        if (!settings.ShowPreviews) {
            clearImages();
        }
    }
    if (menu.Option(T("settings.reload_previews"),
                    { T("settings.reload_previews.detail") })) {
        clearImages();
    }
    if (menu.Option(T("settings.reload_user_dlc"),
                    { T("settings.reload_user_dlc.detail") })) {
        reloadUserDlc();
    }
    if (menu.Option(T("settings.clean_previews"),
                    { T("settings.clean_previews.detail1"), T("settings.clean_previews.detail2") })) {
        clearImages();
        cleanImageDirectory(true);
    }
    if (settings.Persistent) {
        if (menu.Option(T("settings.clear_persistence"), { T("settings.clear_persistence.detail"),
                            gLanguage.Format("settings.persistent_count", { { "count", std::to_string(g_persistentVehicles.size()) } }) })) {
            clearPersistentVehicles();
        }
    }
}

void update_officialdlcmergedmenu(const std::set<std::string>& categories) {
    menu.Title(T("menu.official_dlc.title"));
    menu.Subtitle(T("menu.official_dlc.merged"));

    for (const auto& category : categories) {
        std::string categoryName = FormatCategoryName(category);
        menu.MenuOption(categoryName, "dlc_" + category);
    }
}

void update_officialdlcmenu() {
    menu.Title(T("menu.official_dlc.title"));
    menu.Subtitle(T("menu.official_dlc.sort"));

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
    menu.Title(T("menu.user_dlc.title"));
    menu.Subtitle(T("menu.user_dlc.subtitle"));

    for (const auto& dlc : g_userDlcs) {
        menu.MenuOption(dlc.Name, dlc.Name);
    }
}

void update_perdlcmenu(const DLCDefinition& dlc, const std::set<std::string>& dlcCats) {
    menu.Title(dlc.Name);
    menu.Subtitle(T("menu.official_dlc.sort"));

    for (const auto& category : dlcCats) {
        std::string categoryName = FormatCategoryName(category);
        menu.MenuOption(categoryName, dlc.Name + " " + category);
    }
    if (dlcCats.empty()) {
        menu.Option(T("menu.dlc.unavailable"), {
            gLanguage.Format("menu.dlc.unavailable.detail", { { "dlc", dlc.Name } }),
            gLanguage.Format("menu.dlc.game_version", { { "version", Versions::GetName(getGameVersion()) } })
        });
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
            update_spawnmenu(category, g_addonVehicles, T("origin.addon_vehicles"), settings.CategorizeMake);
        }
    }

    if (settings.MergeDLCs) {
        const std::set<std::string>& categories = settings.CategorizeMake ? g_dlcMakes : g_dlcClasses;

        if (menu.CurrentMenu("officialdlcmergedmenu")) {
            update_officialdlcmergedmenu(categories);
        }
        for (const auto& category : categories) {
            if (menu.CurrentMenu("dlc_" + category)) {
                update_spawnmenu(category, g_dlcVehicles, T("origin.original_all_dlcs"), settings.CategorizeMake);
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
