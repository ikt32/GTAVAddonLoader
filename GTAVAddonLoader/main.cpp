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
#include "NativeMemory.hpp"

extern unsigned long g_StartTime;

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved) {
	std::string logFile = Paths::GetModuleFolder(hInstance) + modDir +
		"\\" + Paths::GetModuleNameWithoutExtension(hInstance) + ".log";
	logger.SetFile(logFile);
	Paths::SetOurModuleHandle(hInstance);

	switch (reason) {
	case DLL_PROCESS_ATTACH:
		scriptRegister(hInstance, ScriptMain);
        DisableThreadLibraryCalls(hInstance);
	    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)initInitVehicleArchetypeHooks, 0, 0, 0);
		logger.Clear();
		logger.Write("GTAVAddonSpawner " + std::string(DISPLAY_VERSION));
		logger.Write("Game version " + eGameVersionToString(getGameVersion()));
		logger.Write("Script registered");
        g_StartTime = GetTickCount();
		break;
	case DLL_PROCESS_DETACH:
		scriptUnregister(hInstance);
        deinitInitVehicleArchetypeHooks();
		break;
	}
	return TRUE;
}
