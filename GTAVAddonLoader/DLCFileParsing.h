#pragma once
#include "inc/types.h"
#include "Util/Util.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

// Small shared helpers for parsing ".list" DLC definition files
// (plain vehicle model names, one per line, hashed with joaat()).
// Used by both UserDLC.cpp and GameDLC.cpp.
namespace DLCFileParsing {
    inline std::string ToLower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }

    inline std::vector<Hash> ParseHashListFile(const std::filesystem::directory_entry& de) {
        std::vector<Hash> hashes;
        std::ifstream file(de.path().string());

        std::string line;
        while (std::getline(file, line)) {
            hashes.emplace_back(joaat(line));
        }
        return hashes;
    }
}
