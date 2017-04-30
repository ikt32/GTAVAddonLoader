#pragma once
#include <vector>

class MenuControls;
class Menu;

class Settings
{
public:
	Settings();
	~Settings();
	void ReadSettings(MenuControls *control, Menu *menuOpts);
	void SaveSettings();
	void SetFiles(const std::string &general, const std::string &menu);

	bool SpawnInside = false;
private:
	std::string settingsGeneralFile;
	std::string settingsMenuFile;
};
