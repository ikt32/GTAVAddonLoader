#pragma once
#include <vector>
#include <simpleini/SimpleIni.h>

namespace NativeMenu {
class Menu;
class MenuControls;
}

class Settings
{
public:
	Settings();
	~Settings();
	void ReadSettings();
	void SaveSettings();
	void SetFiles(const std::string &general);

	bool SpawnInside = false;
	bool SpawnByName = false;
	bool ListAllDLCs = false;
	bool MergeDLCs = false;
	bool Persistent = false;
	bool CategorizeMake = false;
	bool SpawnInplace = false;
	bool SearchMenu = false;
	long SearchCategory = 0;

private:
	std::string settingsGeneralFile;
	std::string settingsMenuFile;
};
