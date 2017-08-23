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

std::vector<Hash> g_missingImages;
std::vector<Vehicle> g_persistentVehicles;

std::set<std::string> g_addonClasses;
std::set<std::string> g_addonMakes;
std::vector<ModelInfo> g_addonVehicles;	// all add-on vehicles
std::vector<AddonImage> g_addonImages;
std::vector<AddonImageMeta> g_addonImageMetadata;
std::vector<std::string> g_addonImageNames; // just all filenames separately for hashing in begin

std::vector<Hash> GameVehicles;             // all base vehicles
std::vector<DLC> g_dlcs;
std::set<std::string> g_dlcClasses;
std::set<std::string> g_dlcMakes;
std::vector<ModelInfo> g_dlcVehicles;
std::vector<SpriteInfo> g_dlcSprites;
std::vector<SpriteInfo> g_dlcSpriteOverrides;

/*
 * Some sprites don't match up with the actual vehicle or don't have the
 * same name as the model. We'll go through this (small) list first.
 * R* pls more consistent pls
 */
void addVehicleSpriteOverrides() {
	// No DLC
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_dlc_import_export", "elegy_a", joaat("elegy2"), 1024, 512)); // prettier
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_jan2016", "banshee2_a", joaat("banshee"), 1024, 512)); // prettier
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_jan2016", "sultan2_a", joaat("sultan"), 1024, 512)); // prettier
	g_dlcSpriteOverrides.push_back(SpriteInfo("candc_truck", "cab_0", joaat("phantom3"), 256, 256));
	g_dlcSpriteOverrides.push_back(SpriteInfo("candc_truck", "cab_1", joaat("hauler2"), 256, 256));

	g_dlcSpriteOverrides.push_back(SpriteInfo("dock_default", "dinghy3", joaat("dinghy"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("dock_default", "dinghy3", joaat("dinghy2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("dock_default", "seashark", joaat("seashark2"), 1024, 512)); // the inaccurate but ok
	g_dlcSpriteOverrides.push_back(SpriteInfo("dock_default", "tropic", joaat("tropic2"), 1024, 512));
	// submersible rip
	// predator rip


	// Returning player
	g_dlcSpriteOverrides.push_back(SpriteInfo("sssa_dlc_mp_to_sp", "dukes", joaat("dukes"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("sssa_dlc_mp_to_sp", "stallion", joaat("stalion"), 1024, 512)); // wtf
	g_dlcSpriteOverrides.push_back(SpriteInfo("dock_default", "sub2", joaat("submersible2"), 1024, 512)); // wtf kraken
	// blimp2 rip
	// blista3 rip (go go monkey)

	// Beach Bum is ok

	// Valentines day massacre
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_valentines", "roosevelt", joaat("btype"), 512, 256));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_valentines2", "roosevelt2", joaat("btype3"), 512, 256));

	// The Business Update is ok
	// The High Life is ok
	// Not a Hipster is ok
	// Independence Day is ok
	// Flight school is ok

	// LTS
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_lts_creator", "furore", joaat("furoregt"), 512, 256));

	// Festive surprise / Christmas2
	g_dlcSpriteOverrides.push_back(SpriteInfo("sssa_dlc_christmas_2", "rloader2", joaat("ratloader2"), 1024, 512)); // wrong but better than nothing

	// Heists
	g_dlcSpriteOverrides.push_back(SpriteInfo("candc_default", "barracks", joaat("barracks3"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("sssa_dlc_christmas_2", "slamvan", joaat("slamvan2"), 1024, 512)); // wrong but better than nothing
	// trash2 missing
	// tanker2 missing

	// Ill Gotten Gains Pt1 is ok
	// Ill Gotten Gains Pt2 is ok

	// Lowriders
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_default", "buccaneer2_a", joaat("buccaneer"), 1024, 512));	// no dlc
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_default", "buccaneer2_b", joaat("buccaneer2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_default", "chino2_a", joaat("chino"), 1024, 512));	// no dlc
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_default", "chino2_b", joaat("chino2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_default", "faction2_a", joaat("faction"), 1024, 512));	// no dlc
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_default", "faction2_b", joaat("faction2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_default", "moonbeam2_a", joaat("moonbeam"), 1024, 512));	// no dlc
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_default", "moonbeam2_b", joaat("moonbeam2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_default", "primo2_a", joaat("primo"), 1024, 512));	// no dlc
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_default", "primo2_b", joaat("primo2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_default", "voodoo_a", joaat("voodoo"), 1024, 512));	// no dlc
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_default", "voodoo_b", joaat("voodoo2"), 1024, 512));


	// Halloween is ok

	// Executives and Other Criminals / apartments
	g_dlcSpriteOverrides.push_back(SpriteInfo("dock_default", "dinghy3", joaat("dinghy4"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("dock_default", "seashark", joaat("seashark3"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("dock_default", "speeder", joaat("speeder2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("dock_default", "toro", joaat("toro2"), 1024, 512));


	g_dlcSpriteOverrides.push_back(SpriteInfo("elt_dlc_apartments", "svolito", joaat("supervolito"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("elt_dlc_apartments", "svolito2", joaat("supervolito2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("candc_default", "valkyrie", joaat("valkyrie2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("candc_default", "cargobob", joaat("cargobob4"), 1024, 512));


	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "baller3_web_vehicle_regular_b", joaat("baller3"), 512, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "baller3_web_vehicle_armoured_b", joaat("baller5"), 512, 512));														 
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "baller4_web_vehicle_regular_b", joaat("baller4"), 512, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "baller4_web_vehicle_armoured_b", joaat("baller6"), 512, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "cog55_web_vehicle_regular_b", joaat("cog55"), 512, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "cog55_web_vehicle_armoured_b", joaat("cog552"), 512, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "cognosc_web_vehicle_regular_b", joaat("cognoscenti"), 512, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "cognosc_web_vehicle_armoured_b", joaat("cognoscenti2"), 512, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "schafter3_web_vehicle_regular_b", joaat("schafter3"), 512, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "schafter3_web_vehicle_armoured_b", joaat("schafter5"), 512, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "schafter4_web_vehicle_regular_b", joaat("schafter4"), 512, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "schafter4_web_vehicle_armoured_b", joaat("schafter6"), 512, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "niteshad", joaat("nightshade"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_apartments", "verlier", joaat("verlierer2"), 1024, 512));


	// Drop zone
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_jan2016", "banshee2_b", joaat("banshee2"), 1024, 512)); // correct custom version
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_jan2016", "sultan2_b", joaat("sultanrs"), 1024, 512)); // correct custom version


	// Lowriders Custom Classics Lowriders2
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_lowrider2", "faction3_b", joaat("faction3"), 1024, 512)); // correct custom version
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_lowrider2", "minivan2_b", joaat("minivan2"), 1024, 512)); // correct custom version
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_lowrider2", "sabregt2_b", joaat("sabregt2"), 1024, 512)); // correct custom version
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_lowrider2", "slamvan3_b", joaat("slamvan3"), 1024, 512)); // correct custom version
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_lowrider2", "tornado5_b", joaat("tornado5"), 1024, 512)); // correct custom version
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_lowrider2", "virgo2_b", joaat("virgo2"), 1024, 512)); // correct custom version
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_lowrider2", "virgo2_a", joaat("virgo3"), 1024, 512)); // correct custom version

	// Finance and Felony / Executive1
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_executive1", "xls_web_vehicle_regular_b", joaat("xls"), 512, 512)); // matches xls2
	g_dlcSpriteOverrides.push_back(SpriteInfo("lgm_dlc_executive1", "xls_web_vehicle_armoured_b", joaat("xls2"), 512, 512));

	// Cunning Stunts
	g_dlcSpriteOverrides.push_back(SpriteInfo("sssa_dlc_stunt", "trophy", joaat("trophytruck"), 512, 256));
	g_dlcSpriteOverrides.push_back(SpriteInfo("sssa_dlc_stunt", "trophy2", joaat("trophytruck2"), 512, 256));

	// Bikers is good

	// Import/Export
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_dlc_import_export", "comet3_b", joaat("comet3"), 1024, 512)); // correct custom version
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_dlc_import_export", "elegy_b", joaat("elegy"), 1024, 512)); // correct custom version
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_dlc_import_export", "specter2_a", joaat("specter"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_dlc_import_export", "specter2_b", joaat("specter2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_dlc_import_export", "italigtb2_a", joaat("italigtb"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_dlc_import_export", "italigtb2_b", joaat("italigtb2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_dlc_import_export", "nero2_a", joaat("nero"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_dlc_import_export", "nero2_b", joaat("nero2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_dlc_import_export", "diablous2_a", joaat("diablous"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_dlc_import_export", "diablous2_b", joaat("diablous2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_dlc_import_export", "fcr2_a", joaat("fcr"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("lsc_dlc_import_export", "fcr2_b", joaat("fcr2"), 1024, 512));
	g_dlcSpriteOverrides.push_back(SpriteInfo("candc_importexport", "wastlndr", joaat("wastelander"), 512, 256));

	// Cunning Stunts 2 is good

	// Gunrunning
	g_dlcSpriteOverrides.push_back(SpriteInfo("candc_default", "insurgent", joaat("insurgent3"), 256, 128)); // pick-up
	g_dlcSpriteOverrides.push_back(SpriteInfo("candc_default", "technical", joaat("technical3"), 256, 128)); // same-ish
	g_dlcSpriteOverrides.push_back(SpriteInfo("candc_gunrunning", "trsmall2", joaat("trailersmall2"), 512, 256));
	g_dlcSpriteOverrides.push_back(SpriteInfo("candc_truck", "cab_0", joaat("phantom3"), 256, 256));
	g_dlcSpriteOverrides.push_back(SpriteInfo("candc_truck", "cab_1", joaat("hauler2"), 256, 256));
	g_dlcSpriteOverrides.push_back(SpriteInfo("candc_truck", "thumbnail", joaat("trailerlarge"), 256, 128));
	g_dlcSpriteOverrides.push_back(SpriteInfo("foreclosures_bunker", "transportationb_2", joaat("caddy3"), 512, 256));
}

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
	if (g_dlcSprites.size() == expectedPreviewSprites)
		return;

	for (auto dict : WebsiteDicts) {
		GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT((char*)dict.c_str(), false);
	}
	for (auto dict : WebsiteDicts) {
		GRAPHICS::REQUEST_STREAMED_TEXTURE_DICT((char*)dict.c_str(), false);
		auto textures = MemoryAccess::GetTexturesFromTxd(joaat(dict));
		for (auto texture : textures) {
			SpriteInfo thing;
			if (!isHashInImgVector(joaat(texture->name), g_dlcSprites, &thing)) {
				g_dlcSprites.push_back(SpriteInfo(dict, texture->name, joaat(texture->name), texture->resolutionX, texture->resolutionY));
			}
		}
	}
	logger.Write("Found " + std::to_string(g_dlcSprites.size()) + " preview sprites (dict)");
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

	std::string gxtName = getGxtName(hash);
	if (joaat(gxtName) == hash) return gxtName;

	gxtName = removeSpecialChars(gxtName);
	if (joaat(gxtName) == hash) return gxtName;

	return "NOTFOUND";
}

// sorting thing
bool predicateHashByName(Hash h1, Hash h2) {
	std::string name1 = getGxtName(h1);
	std::string name2 = getGxtName(h2);
	return name1 < name2;
}

// sorting thing 2
bool predicateAddonVehicleHashByName(ModelInfo a1, ModelInfo a2) {
	std::string name1 = getGxtName(a1.ModelHash);
	std::string name2 = getGxtName(a2.ModelHash);
	if (name1 == name2) {
		return guessModelName(a1.ModelHash) < guessModelName(a2.ModelHash);
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
			std::string makeName = UI::_GET_LABEL_TEXT(MemoryAccess::GetVehicleMakeName(hash));
			dlc.Vehicles.push_back(ModelInfo(className, makeName, hash));
			dlc.Classes.emplace(className);
			dlc.Makes.emplace(makeName);
		}
	}
	g_dlcVehicles.clear();
	g_dlcClasses.clear();
	for (auto dlc : g_dlcs) {
		for (ModelInfo vehicle : dlc.Vehicles) {
			g_dlcVehicles.push_back(vehicle);
		}
		for (auto dlcClass : dlc.Classes) {
			g_dlcClasses.emplace(dlcClass);
		}
		for (auto dlcMake : dlc.Makes) {
			g_dlcMakes.emplace(dlcMake);
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
			logStream << std::left << std::setw(nameLength) << std::setfill(' ') << getGxtName(hash);

			logger.Write(logStream.str());

			std::string makeName = UI::_GET_LABEL_TEXT(MemoryAccess::GetVehicleMakeName(hash));

			g_addonVehicles.push_back(ModelInfo(className, makeName, hash));
			g_addonClasses.emplace(className);
			g_addonMakes.emplace(makeName);
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

		showSubtitle("Spawned " + getGxtName(hash) + " (" + guessModelName(hash) + ")");
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

std::vector<std::string> resolveVehicleInfo(std::vector<ModelInfo>::value_type addonVehicle) {
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

	AddonImage addonImage;
	SpriteInfo spriteInfo;
	if (isHashInImgVector(addonVehicle.ModelHash, g_addonImages, &addonImage)) {
		extras.push_back(menu.ImagePrefix + std::to_string(addonImage.TextureID) +
			"W" + std::to_string(addonImage.ResX) +
			"H" + std::to_string(addonImage.ResY));
	}
	else if (isHashInImgVector(addonVehicle.ModelHash, g_dlcSpriteOverrides, &spriteInfo)) {
		extras.push_back(menu.SpritePrefix +
			spriteInfo.Dict + " " +
			spriteInfo.Name + " " +
			"W" + std::to_string(spriteInfo.ResX) +
			"H" + std::to_string(spriteInfo.ResY));
	}
	else if (std::find(GameVehicles.begin(), GameVehicles.end(), addonVehicle.ModelHash) != GameVehicles.end() &&
			isHashInImgVector(addonVehicle.ModelHash, g_dlcSprites, &spriteInfo)) {
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

		if (std::find(g_missingImages.begin(), g_missingImages.end(), addonVehicle.ModelHash) == g_missingImages.end()) {
			resolveImage(addonVehicle.ModelHash);
		}
	}

	char* makeName = MemoryAccess::GetVehicleMakeName(addonVehicle.ModelHash);
	extras.push_back("Make name: \t" + std::string(UI::_GET_LABEL_TEXT(makeName)) + " (" + std::string(makeName) + ")");
	extras.push_back("Model name: \t" + guessModelName(addonVehicle.ModelHash));
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

	if (MemoryAccess::findPatterns())
		MemoryAccess::enableCarsGlobal();
	
	MemoryAccess::initTxdStore();
	g_dlcs = buildDLClist();
	buildBlacklist();
	storeImageNames();
	cacheAddons();
	cacheDLCs();
	addVehicleSpriteOverrides();

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
