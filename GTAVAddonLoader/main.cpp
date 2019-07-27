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

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved) {
    std::string logFile = Paths::GetModuleFolder(hInstance) + modDir +
        "\\" + Paths::GetModuleNameWithoutExtension(hInstance) + ".log";
    logger.SetFile(logFile);
    Paths::SetOurModuleHandle(hInstance);

    switch (reason) {
    case DLL_PROCESS_ATTACH:
        logger.Clear();
        logger.Write(INFO, "GTAVAddonSpawner " + std::string(DISPLAY_VERSION));
        logger.Write(INFO, "Game version " + eGameVersionToString(getGameVersion()));
        scriptRegister(hInstance, ScriptMain);
        setupHooks();
        logger.Write(INFO, "Script registered");
        break;
    case DLL_PROCESS_DETACH:
        scriptUnregister(hInstance);
        break;
    }
    return TRUE;
}
