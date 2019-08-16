#include "UserDLC.h"
#include "Util/Paths.h"
#include "script.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include "Util/Util.hpp"

namespace fs = std::filesystem;

namespace {
    // Serious CBA.
    std::string toLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }
}

DLC parseFile(const std::filesystem::directory_entry& de) {
    std::vector<Hash> hashes;
    std::ifstream file(de.path().string());

    std::string line;
    while (std::getline(file, line)) {
        hashes.emplace_back(joaat(line));
    }

    if (hashes.empty()) {
        return DLC("", std::vector<Hash>());
    }

    return DLC(de.path().stem().string(), hashes);
}

std::vector<DLC> BuildUserDLCList() {
    std::vector<DLC> userDLCs;
    std::string userDlcPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\UserDLC";
    for (auto& file : fs::directory_iterator(userDlcPath)) {
        if (toLower(fs::path(file).extension().string()) != ".list")
            continue;

        DLC dlc = parseFile(file);
        if (!dlc.Name.empty())
            userDLCs.emplace_back(dlc);
    }
    return userDLCs;
}
