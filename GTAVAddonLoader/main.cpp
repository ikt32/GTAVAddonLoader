/*
THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
http://dev-c.com
(C) Alexander Blade 2015
*/

#include "..\..\ScriptHookV_SDK\inc\main.h"
#include "script.h"
#include "keyboard.h"
#include "Util/Paths.h"
#include "Util/Logger.hpp"
#include "Util/Versions.h"

#include "dirent.h"

std::vector<std::string> dlcpackFolders;

bool scandlcpacks(HMODULE hInstance) {
	dlcpackFolders.clear();

	DIR *dir;
	struct dirent *ent;
	std::string dlcpacksdir = Paths::GetModuleFolder(hInstance) + "\\mods\\update\\x64\\dlcpacks\\";
	if ((dir = opendir(dlcpacksdir.c_str())) != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_type == DT_DIR) {
				dlcpackFolders.push_back(ent->d_name);
			}
		}
		closedir(dir);
		return true;
	}
	/* could not open directory */
	perror("");
	return false;
}

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved)
{
	std::string logFile = Paths::GetModuleFolder(hInstance) + modDir +
		"\\" + Paths::GetModuleNameWithoutExtension(hInstance) + ".log";
	logger.SetFile(logFile);
	Paths::SetOurModuleHandle(hInstance);
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		scriptRegister(hInstance, ScriptMain);
		logger.Clear();
		logger.Write("GTAVAddonSpawner " + std::string(DISPLAY_VERSION));
		logger.Write("Game version " + eGameVersionToString(getGameVersion()));
		logger.Write("Script registered");
		if (scandlcpacks(hInstance)) {
			logger.Write(("dlcpacks scan success: " + std::to_string(dlcpackFolders.size()) + " folders").c_str());
		}
		else {
			logger.Write("dlcpacks scan failed! Not using a mods folder?");
		}
		break;
	case DLL_PROCESS_DETACH:
		scriptUnregister(hInstance);
		joinRemainingThreads();
		break;
	}
	return TRUE;
}
