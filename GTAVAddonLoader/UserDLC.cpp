#include "UserDLC.h"
#include "DLCFileParsing.h"
#include "Util/Paths.h"
#include "script.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include "Util/Util.hpp"

namespace fs = std::filesystem;

DLCDefinition parseFile(const std::filesystem::directory_entry& de) {
    std::vector<Hash> hashes = DLCFileParsing::ParseHashListFile(de);

    if (hashes.empty()) {
        return DLCDefinition("", std::vector<Hash>());
    }

    return DLCDefinition(de.path().stem().string(), hashes);
}

std::vector<DLCDefinition> BuildUserDLCList() {
    std::vector<DLCDefinition> userDLCs;
    std::string userDlcPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\UserDLC";
    for (auto& file : fs::directory_iterator(userDlcPath)) {
        if (DLCFileParsing::ToLower(fs::path(file).extension().string()) != ".list")
            continue;

        DLCDefinition dlc = parseFile(file);
        if (!dlc.Name.empty())
            userDLCs.emplace_back(dlc);
    }
    return userDLCs;
}
