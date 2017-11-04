/*
THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
http://dev-c.com
(C) Alexander Blade 2015
*/

#pragma once

#define DISPLAY_VERSION "v1.3.1"

#include <string>
#include <vector>
#include <inc/types.h>
#include "ExtraTypes.h"

const std::string modDir  = "\\AddonSpawner";

void ScriptMain();

void cacheAddons();
void cacheDLCs();
void resolveVehicleSpriteInfo();
void update_menu();
void onMenuOpen();
void onMenuExit();
void storeImageNames();
void spawnVehicle(Hash hash);
std::vector<std::string> resolveVehicleInfo(std::vector<ModelInfo>::value_type addonVehicle);

void clearPersistentVehicles();
void cleanImageDirectory(bool backup);
std::string guessModelName(Hash hash);

//extern std::vector<std::string> dlcpackFolders;
