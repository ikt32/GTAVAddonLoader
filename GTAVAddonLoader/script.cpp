#include "script.h"

#include <array>
#include <experimental/filesystem>
#include <future>
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
#include "Util/Versions.h"

#include "keyboard.h"
#include "settings.h"
#include "VehicleHashes.h"

namespace fs = std::experimental::filesystem;

// nameHash, textureId, width, height
using AddonImage = std::tuple<Hash, int, int, int>;

// filePath, width, height
using AddonImageMeta = std::tuple<std::string, int, int>;

// className, vehicleHash
using AddonVehicle = std::pair<std::string, Hash>;

class SpriteInfo {
public:
	SpriteInfo(): HashedName(0), ResX(0), ResY(0) {}
	SpriteInfo(std::string dict, std::string name, Hash hash, uint16_t resX, uint16_t resY) :
			   Dict(dict), Name(name), HashedName(hash), ResX(resX), ResY(resY) { }

	std::string Dict;
	std::string Name;
	Hash HashedName;
	uint16_t ResX;
	uint16_t ResY;
};

NativeMenu::Menu menu;
NativeMenu::MenuControls controls;
Settings settings;

std::string settingsGeneralFile;
std::string settingsMenuFile;

Player player;
Ped playerPed;

int prevNotification;

std::vector<Hash> Vehicles;

std::vector<AddonVehicle> g_addonVehicles;
std::set<std::string> g_addonClasses;

std::vector<AddonVehicle> g_dlcVehicles;
std::set<std::string> g_dlcClasses;

std::vector<AddonImage> g_addonImages;
std::vector<AddonImageMeta> g_addonImageMetadata;

std::vector<std::future<void>> futures;
std::vector<std::thread> threads;

int noImageHandle;

std::vector<SpriteInfo> g_spriteInfos;

class DLC {
public:
	DLC(std::string name, std::vector<Hash> hashes) :
	Name(name), Hashes(hashes)
	{ }
	std::string Name;
	std::set<std::string> Classes;
	std::vector<Hash> Hashes;
	std::vector<std::pair<std::string, Hash>> Vehicles;
};

std::vector<DLC> dlcs;

Hash joaat(std::string s) {
	std::transform(s.begin(), s.end(), s.begin(), ::tolower);

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
	g_addonImageMetadata.clear();
	std::string imgPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\img";
	for (auto &file : fs::directory_iterator(imgPath)) {
		std::stringstream fileName;
		int width;
		int height;
		fileName << file;
		if (GetIMGDimensions(fileName.str(), &width, &height)) {
			g_addonImageMetadata.push_back(std::make_tuple(fileName.str(), width, height));
		}
	}
}

bool isPresentinAddonImages(Hash hash, AddonImage *addonImageResult) {
	auto addonImage = std::find_if(g_addonImages.begin(), g_addonImages.end(), [&hash](const AddonImage& element) {
		return std::get<0>(element) == hash;
	});
	if (g_addonImages.end() != addonImage) {
		if (addonImageResult != nullptr) 
			*addonImageResult = *addonImage;
		return true;
	}
	return false;
}

bool isPresentinTextures(Hash hash, SpriteInfo * spriteResult) {
	auto sprite = std::find_if(g_spriteInfos.begin(), g_spriteInfos.end(), [&hash](const SpriteInfo& element) {
		return element.HashedName == hash;
	});
	if (g_spriteInfos.end() != sprite) {
		*spriteResult = *sprite;
		return true;
	}
	return false;
}

// Uses createTexture()
void resolveImgs2() {
	for (auto metadata : g_addonImageMetadata) {
		auto fileName = std::get<0>(metadata);
		Hash hash = joaat(fs::path(fileName).stem().string());
		if (isPresentinAddonImages(hash, nullptr)) {
			continue;
		}
		auto width = std::get<1>(metadata);
		auto height = std::get<2>(metadata);
		int handle = createTexture(fileName.c_str());
		if (hash == joaat("noimage"))
			noImageHandle = handle;
		g_addonImages.push_back(std::make_tuple(hash, handle, width, height));
	}
	logger.Write("Found " + std::to_string(g_addonImages.size()) + " preview images.");
}

void checkThread() {
	for (auto it = futures.begin(); it != futures.end();) {
		auto status = (*it).wait_for(std::chrono::milliseconds(0));
		if (status == std::future_status::ready) {
			(*it).get();
			resolveImgs2();
			futures.erase(it);
		}
		else {
			++it;
		}
	}
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

// use better names, ikt.
bool predicateAddonVehicleHashByName(AddonVehicle a1, AddonVehicle a2) {
	std::string name1 = prettyNameFromHash(a1.second);
	std::string name2 = prettyNameFromHash(a2.second);
	if (name1 == name2) {
		return guessModelName(a1.second) < guessModelName(a2.second);
	}
	return name1 < name2;
}

void updateSettings() {
	settings.SaveSettings();
	settings.ReadSettings();
	menu.ReadSettings();
}

void sortDLCVehicles() {
	for (auto &dlc : dlcs) {
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
	for (auto dlc : dlcs) {
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
	dlcs = {
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
	sortDLCVehicles();
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
	for (auto dlc : dlcs) {
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
	if (isPresentinAddonImages(addonVehicle.second, &addonImage)) {
		extras.push_back(menu.ImagePrefix + std::to_string(std::get<1>(addonImage)) +
			"W" + std::to_string(std::get<2>(addonImage)) +
			"H" + std::to_string(std::get<3>(addonImage)));
	}
	else if (hashIt != Vehicles.end() && isPresentinTextures(addonVehicle.second, &spriteInfo)) {
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

void spawnMenu(std::string className, std::vector<AddonVehicle> addonVehicles, std::string origin) {
	menu.Title(className);
	menu.Subtitle(origin);
	for (auto addonVehicle : addonVehicles) {
		if (className == addonVehicle.first) {
			char *name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(addonVehicle.second);
			std::string displayName = UI::_GET_LABEL_TEXT(name);
			if (displayName == "NULL") {
				displayName = name;
			}

			std::vector<std::string> extras = {};
			bool visible = false;
			if (menu.OptionPlus(displayName, extras, &visible, nullptr, nullptr, "Add-on info", {})) {
				spawnVehicle(addonVehicle.second);
			}
			if (visible) {
				extras = resolveVehicleInfo(addonVehicle);
				menu.OptionPlusPlus(extras, "Add-on info");
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

void onMenuOpen() {
	updateSettings();

	cacheAddons();
	cacheDLCs();

	resolveVehicleSpriteInfo();

	std::packaged_task<void()> task(&resolveImgs);
	auto f = task.get_future();
	std::thread thread(std::move(task));
	futures.push_back(std::move(f));
	thread.detach();
}

void onMenuExit() {
	manualVehicleName.clear();
}

void update_menu() {
	menu.CheckKeys();

	if (menu.CurrentMenu("mainmenu")) {
		menu.Title("Add-on spawner");
		menu.Subtitle(DISPLAY_VERSION, false);

		menu.MenuOption("Settings","settingsmenu");

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
				menu.MenuOption("Spawn official DLCs", "officialdlcmergedmenu");
			}
			else {
				menu.MenuOption("Spawn official DLCs", "officialdlcmenu");
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
		if (menu.BoolOption("Spawn manually", settings.SpawnByName)) {
			settings.SaveSettings();
		}
		if (menu.BoolOption("List all DLCs", settings.ListAllDLCs, 
			{ "Show all official DLC vehicles."
			  " These will appear in their own submenu, sorted per class, per DLC."})) {
			settings.SaveSettings();
		}
		if (menu.BoolOption("Merge DLCs", settings.MergeDLCs, 
			{ "Don't sort per DLC and just show the vehicles per class."})) {
			settings.SaveSettings();
		}
		if (menu.Option("Reload previews", { "Reload your image previews. Not required when adding previews, "
											 "but useful if previews are changed."})) {
			resolveVehicleSpriteInfo();
			g_addonImages.clear();

			std::packaged_task<void()> task(&resolveImgs);
			auto f = task.get_future();
			std::thread thread(std::move(task));
			futures.push_back(std::move(f));
			thread.detach();
		}
	}

	for (auto className : g_addonClasses) {
		if (menu.CurrentMenu(className)) { spawnMenu(className, g_addonVehicles, "Add-on vehicles"); }
	}

	if (!settings.MergeDLCs) {
		if (menu.CurrentMenu("officialdlcmenu")) {
			menu.Title("Official DLC");
			menu.Subtitle("");

			for (auto dlc : dlcs) {
				menu.MenuOption(dlc.Name, dlc.Name);
			}
		}

		for (auto dlc : dlcs) {
			if (menu.CurrentMenu(dlc.Name)) {
				menu.Title(dlc.Name);
				menu.Subtitle("By DLC");

				for (auto className : dlc.Classes) {
					menu.MenuOption(className, dlc.Name + " " + className);
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
	checkThread();
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

void joinRemainingThreads() {
	for (auto &thread : threads) {
		thread.join();
	}
}
