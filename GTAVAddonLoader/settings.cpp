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
	ListAllDLCs = settingsGeneral.GetBoolValue("OPTIONS", "ListAllDLCs", false);
	MergeDLCs = settingsGeneral.GetBoolValue("OPTIONS", "MergeDLCs", false);
	Persistent = settingsGeneral.GetBoolValue("OPTIONS", "Persistent", false);
	CategorizeMake = settingsGeneral.GetBoolValue("OPTIONS", "CategorizeMake", false);
}


void Settings::SaveSettings() {
	CSimpleIniA settings;
	settings.SetUnicode();
	settings.LoadFile(settingsGeneralFile.c_str());

	settings.SetBoolValue("OPTIONS", "SpawnInside", SpawnInside);
	settings.SetBoolValue("OPTIONS", "SpawnByName", SpawnByName);
	settings.SetBoolValue("OPTIONS", "ListAllDLCs", ListAllDLCs);
	settings.SetBoolValue("OPTIONS", "MergeDLCs", MergeDLCs);
	settings.SetBoolValue("OPTIONS", "Persistent", Persistent);
	settings.SetBoolValue("OPTIONS", "CategorizeMake", CategorizeMake);

	settings.SaveFile(settingsGeneralFile.c_str());
}
