#include "settings.h"
#include "simpleini/SimpleIni.h"
#include "keyboard.h"
#include "menucontrols.h"
#include "menu.h"

Settings::Settings() { }


Settings::~Settings() { }

void Settings::SetFiles(const std::string &general, const std::string &menu) {
	settingsGeneralFile = general;
	settingsMenuFile = menu;
}

void Settings::ReadSettings(NativeMenu::MenuControls *control, NativeMenu::Menu *menuOpts) {

	CSimpleIniA settingsGeneral;
	settingsGeneral.SetUnicode();
	settingsGeneral.LoadFile(settingsGeneralFile.c_str());
	
	SpawnInside = settingsGeneral.GetBoolValue("OPTIONS", "SpawnInside", false);
	

	CSimpleIniA settingsMenu;
	settingsMenu.SetUnicode();
	settingsMenu.LoadFile(settingsMenuFile.c_str());
	control->ControlKeys[NativeMenu::MenuControls::ControlType::MenuKey]	= str2key(settingsMenu.GetValue("MENU", "MenuKey",		"VK_OEM_4"));
	control->ControlKeys[NativeMenu::MenuControls::ControlType::MenuUp]		= str2key(settingsMenu.GetValue("MENU", "MenuUp",		"UP"));
	control->ControlKeys[NativeMenu::MenuControls::ControlType::MenuDown]	= str2key(settingsMenu.GetValue("MENU", "MenuDown",		"DOWN"));
	control->ControlKeys[NativeMenu::MenuControls::ControlType::MenuLeft]	= str2key(settingsMenu.GetValue("MENU", "MenuLeft",		"LEFT"));
	control->ControlKeys[NativeMenu::MenuControls::ControlType::MenuRight]	= str2key(settingsMenu.GetValue("MENU", "MenuRight",	"RIGHT"));
	control->ControlKeys[NativeMenu::MenuControls::ControlType::MenuSelect] = str2key(settingsMenu.GetValue("MENU", "MenuSelect",	"RETURN"));
	control->ControlKeys[NativeMenu::MenuControls::ControlType::MenuCancel] = str2key(settingsMenu.GetValue("MENU", "MenuCancel",	"BACKSPACE"));
#pragma warning(push)
#pragma warning(disable: 4244)
	menuOpts->menux = settingsMenu.GetDoubleValue("MENU", "MenuX", 0.2);
	menuOpts->menuy = settingsMenu.GetDoubleValue("MENU", "MenuY", 0.125);
#pragma warning(pop)
}


void Settings::SaveSettings() {
	CSimpleIniA settings;
	settings.SetUnicode();
	settings.LoadFile(settingsGeneralFile.c_str());

	settings.SetBoolValue("OPTIONS", "SpawnInside", SpawnInside);
	settings.SaveFile(settingsGeneralFile.c_str());
}
