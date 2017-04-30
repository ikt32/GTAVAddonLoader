#include "settings.h"
#include "Menu/controls.h"
#include "Util/simpleini/SimpleIni.h"
#include "keyboard.h"
#include "Menu/MenuClass.h"

Settings::Settings() { }


Settings::~Settings() { }

void Settings::SetFiles(const std::string &general, const std::string &menu) {
	settingsGeneralFile = general;
	settingsMenuFile = menu;
}

void Settings::ReadSettings(MenuControls *control, Menu *menuOpts) {

	CSimpleIniA settingsGeneral;
	settingsGeneral.SetUnicode();
	settingsGeneral.LoadFile(settingsGeneralFile.c_str());
	
	enableMod = settingsGeneral.GetBoolValue("OPTIONS", "EnableMod", false);
	autoApply = settingsGeneral.GetBoolValue("OPTIONS", "AutoApply", false);
	

	CSimpleIniA settingsMenu;
	settingsMenu.SetUnicode();
	settingsMenu.LoadFile(settingsMenuFile.c_str());
	control->ControlKeys[MenuControls::ControlType::MenuKey]	= str2key(settingsMenu.GetValue("MENU", "MenuKey",		"VK_OEM_4"));
	control->ControlKeys[MenuControls::ControlType::MenuUp]		= str2key(settingsMenu.GetValue("MENU", "MenuUp",		"UP"));
	control->ControlKeys[MenuControls::ControlType::MenuDown]	= str2key(settingsMenu.GetValue("MENU", "MenuDown",		"DOWN"));
	control->ControlKeys[MenuControls::ControlType::MenuLeft]	= str2key(settingsMenu.GetValue("MENU", "MenuLeft",		"LEFT"));
	control->ControlKeys[MenuControls::ControlType::MenuRight]	= str2key(settingsMenu.GetValue("MENU", "MenuRight",	"RIGHT"));
	control->ControlKeys[MenuControls::ControlType::MenuSelect] = str2key(settingsMenu.GetValue("MENU", "MenuSelect",	"RETURN"));
	control->ControlKeys[MenuControls::ControlType::MenuCancel] = str2key(settingsMenu.GetValue("MENU", "MenuCancel",	"BACKSPACE"));
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

	settings.SetBoolValue("OPTIONS", "EnableMod", enableMod);
	settings.SetBoolValue("OPTIONS", "AutoApply", autoApply);
	settings.SaveFile(settingsGeneralFile.c_str());
}
