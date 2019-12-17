/*
THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
http://dev-c.com
(C) Alexander Blade 2015
*/

#pragma once

#define DISPLAY_VERSION "v1.4.1"

#include <string>
#include <vector>
#include <inc/types.h>
#include "ExtraTypes.h"

const std::string modDir  = "\\AddonSpawner";

void ScriptMain();

void update_menu();
void onMenuOpen();
void onMenuExit();
std::vector<std::string> resolveVehicleInfo(const ModelInfo& addonVehicle);

void cacheAddons();
void cacheDLCs();

void clearImages();
void clearPersistentVehicles();
void cleanImageDirectory(bool backup);
void reloadUserDlc();

void spawnVehicle(Hash hash);
