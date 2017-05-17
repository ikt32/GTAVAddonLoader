#pragma once
#include <vector>

namespace NativeMenu {
class Menu;
class MenuControls;
}

class Settings
{
public:
	Settings();
	~Settings();
	void ReadSettings(NativeMenu::MenuControls *control, NativeMenu::Menu *menuOpts);
	void SaveSettings();
	void SetFiles(const std::string &general, const std::string &menu);

	bool SpawnInside = false;
	bool SpawnByName = false;
private:
	std::string settingsGeneralFile;
	std::string settingsMenuFile;
};
