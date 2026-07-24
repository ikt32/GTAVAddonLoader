#include "GameDLC.h"
#include "DLCFileParsing.h"
#include "Util/Paths.h"
#include "script.h"

#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

namespace {
    struct GameDLCEntry {
        std::string SortKey;
        DLCDefinition Definition;
    };

    bool nameExistsCaseInsensitive(const std::vector<DLCDefinition>& dlcs, const std::string& name) {
        std::string nameLower = DLCFileParsing::ToLower(name);
        return std::find_if(dlcs.begin(), dlcs.end(), [&](const DLCDefinition& d) {
            return DLCFileParsing::ToLower(d.Name) == nameLower;
        }) != dlcs.end();
    }

    // Splits a filename stem "<key>_<Name>" into { key, Name }. If no
    // underscore is present, the whole stem is used as the name and the
    // key is left empty (sorts first/lowest priority).
    std::pair<std::string, std::string> splitKeyAndName(const std::string& stem) {
        auto pos = stem.find('_');
        if (pos == std::string::npos) {
            return { std::string(), stem };
        }
        return { stem.substr(0, pos), stem.substr(pos + 1) };
    }
}

std::vector<DLCDefinition> BuildGameDLCList(const std::vector<DLCDefinition>& existingDlcs) {
    std::string gameDlcPath = Paths::GetModuleFolder(Paths::GetOurModuleHandle()) + modDir + "\\GameDLC";

    if (!fs::exists(gameDlcPath)) {
        return {};
    }

    std::vector<GameDLCEntry> entries;
    std::vector<DLCDefinition> seenGameDlcs;
    for (auto& file : fs::directory_iterator(gameDlcPath)) {
        if (DLCFileParsing::ToLower(fs::path(file).extension().string()) != ".list")
            continue;

        auto [key, name] = splitKeyAndName(fs::path(file).stem().string());
        if (name.empty())
            continue;

        // An official script update transparently supersedes a user-defined
        // GameDLC entry of the same name, and duplicate GameDLC entries
        // (e.g. two files resolving to the same name) are skipped too.
        if (nameExistsCaseInsensitive(existingDlcs, name) || nameExistsCaseInsensitive(seenGameDlcs, name))
            continue;

        std::vector<Hash> hashes = DLCFileParsing::ParseHashListFile(file);
        if (hashes.empty())
            continue;

        DLCDefinition dlc(name, hashes);
        seenGameDlcs.push_back(dlc);
        entries.push_back({ key, std::move(dlc) });
    }

    std::stable_sort(entries.begin(), entries.end(), [](const GameDLCEntry& a, const GameDLCEntry& b) {
        return a.SortKey < b.SortKey;
    });

    std::vector<DLCDefinition> gameDLCs;
    gameDLCs.reserve(entries.size());
    for (auto& entry : entries) {
        gameDLCs.push_back(std::move(entry.Definition));
    }
    return gameDLCs;
}
