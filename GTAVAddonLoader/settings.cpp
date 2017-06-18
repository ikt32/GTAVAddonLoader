#include "settings.h"
#include "simpleini/SimpleIni.h"

Settings::Settings() { }

Settings::~Settings() { }

void Settings::SetFiles(const std::string &general) {
	settingsGeneralFile = general;
}

void Settings::ReadSettings() {

	CSimpleIniA settingsGeneral;
	settingsGeneral.SetUnicode();
	settingsGeneral.LoadFile(settingsGeneralFile.c_str());
	
	SpawnInside = settingsGeneral.GetBoolValue("OPTIONS", "SpawnInside", false);
	SpawnByName = settingsGeneral.GetBoolValue("OPTIONS", "SpawnByName", false);
}


void Settings::SaveSettings() {
	CSimpleIniA settings;
	settings.SetUnicode();
	settings.LoadFile(settingsGeneralFile.c_str());

	settings.SetBoolValue("OPTIONS", "SpawnInside", SpawnInside);
	settings.SaveFile(settingsGeneralFile.c_str());
}
