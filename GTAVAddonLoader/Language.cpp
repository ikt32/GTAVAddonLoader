#include "Language.h"

#include "Util/Logger.hpp"

#include <Windows.h>
#include <Shlwapi.h>
#include <xmllite.h>

#include <algorithm>
#include <cctype>
#include <cwctype>
#include <set>
#include <system_error>

LanguageManager gLanguage;

namespace {
using TStringMap = std::unordered_map<std::string, std::string>;
const TStringMap sEnglishStrings = {
    { "common.no_make", "No make" },
    { "input.delete_backspace", "Use Delete for backspace" },
    { "input.enter_model", "Enter car model:" },
    { "input.enter_name", "Enter name" },
    { "menu.main.title", "Add-on spawner" },
    { "menu.main.settings", "Settings" },
    { "menu.main.search_vehicles", "Search vehicles" },
    { "menu.main.spawn_by_name", "Spawn by name" },
    { "menu.main.spawn_official_dlcs", "Spawn official DLCs" },
    { "menu.main.spawn_user_dlcs", "Spawn user DLCs" },
    { "menu.search.title", "Search" },
    { "menu.search.searching_for", "Searching for:" },
    { "menu.search.scope", "Search in" },
    { "menu.search.scope.game", "Game vehicles" },
    { "menu.search.scope.addon", "Add-on vehicles" },
    { "menu.search.action", "Search for ..." },
    { "menu.search.entry", "Search entry" },
    { "menu.settings.title", "Settings" },
    { "menu.settings.language", "Language" },
    { "settings.spawn_inside", "Spawn inside vehicle" },
    { "settings.spawn_in_place", "Spawn in place" },
    { "settings.spawn_in_place.detail1", "Don't spawn to the right of the previous car, but spawn at the current position. This replaces the current vehicle." },
    { "settings.spawn_in_place.detail2", "Only active if \"Spawn inside vehicle\" is turned on." },
    { "settings.persistence", "Enable persistence" },
    { "settings.persistence.detail", "Spawned cars don't disappear." },
    { "settings.spawn_max_perf_mods", "Spawn tuned" },
    { "settings.spawn_max_perf_mods.detail", "Spawn vehicles with all performance mods (engine, brakes, transmission, suspension, armor, turbo) installed and maximized." },
    { "settings.spawn_by_name", "Spawn by name" },
    { "settings.spawn_by_name.detail1", "Spawn vehicles by their model name." },
    { "settings.adds_main_option", "This setting adds an option to the main menu." },
    { "settings.categorize_make", "Categorize by make" },
    { "settings.categorize_make.detail", "Categorizing by {category}." },
    { "settings.category.make", "make" },
    { "settings.category.class", "class" },
    { "settings.list_all_dlcs", "List all DLCs" },
    { "settings.list_all_dlcs.detail", "Show all official DLC vehicles. These will appear in their own submenu, sorted per class, per DLC." },
    { "settings.merge_dlcs", "Merge DLCs" },
    { "settings.merge_dlcs.detail", "Don't sort per DLC and just show the vehicles per class." },
    { "settings.search_menu", "Enable search menu" },
    { "settings.search_menu.detail", "Search for vehicles by their make, game name or model name." },
    { "settings.show_previews", "Show previews" },
    { "settings.show_previews.detail", "Show or hide vehicle previews" },
    { "settings.reload_previews", "Reload previews" },
    { "settings.reload_previews.detail", "Use for when you changed an image that's already been loaded." },
    { "settings.reload_user_dlc", "Reload user DLC" },
    { "settings.reload_user_dlc.detail", "Reload your custom groupings" },
    { "settings.clean_previews", "Clean up image preview folder" },
    { "settings.clean_previews.detail1", "Remove images from the preview folder that aren't detected as add-ons." },
    { "settings.clean_previews.detail2", "Removed files are put in a \"bak.timestamp\" folder." },
    { "settings.clear_persistence", "Clear persistence" },
    { "settings.clear_persistence.detail", "Clears the persistence on spawned vehicles" },
    { "settings.persistent_count", "Persistent vehicles: {count}" },
    { "vehicle.info.title", "Vehicle info" },
    { "vehicle.info.make", "Make: \t{value}" },
    { "vehicle.info.name", "Name: \t{value}" },
    { "vehicle.info.model", "Model: \t{value}" },
    { "vehicle.info.mod_kits", "Mod kit ID(s): \t{value}" },
    { "common.none", "None" },
    { "menu.official_dlc.title", "Official DLC" },
    { "menu.official_dlc.merged", "Merged" },
    { "menu.official_dlc.sort", "Sort by DLC" },
    { "menu.user_dlc.title", "User DLC" },
    { "menu.user_dlc.subtitle", "User add-on groupings" },
    { "menu.dlc.unavailable", "DLC unavailable." },
    { "menu.dlc.unavailable.detail", "The dlclist.xml and/or game files do not contain the '{dlc}' content." },
    { "menu.dlc.game_version", "Game version: {version}" },
    { "origin.addon_vehicles", "Add-on vehicles" },
    { "origin.original_all_dlcs", "Original + All DLCs" },
    { "subtitle.model_load_failed", "Couldn't load model" },
    { "subtitle.vehicle_spawned", "Spawned {vehicle} ({model})" },
    { "subtitle.vehicle_missing", "Vehicle doesn't exist" },
};

std::string WideToUtf8(const wchar_t* value, unsigned int length) {
    if (!value || length == 0)
        return {};

    int required = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, static_cast<int>(length),
                                       nullptr, 0, nullptr, nullptr);
    if (required <= 0)
        return {};

    std::string result(required, '\0');
    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, static_cast<int>(length),
                            result.data(), required, nullptr, nullptr) <= 0) {
        return {};
    }
    return result;
}

std::string Trim(std::string value) {
    const auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), isSpace));
    value.erase(std::find_if_not(value.rbegin(), value.rend(), isSpace).base(), value.end());
    return value;
}

std::set<std::string> Placeholders(const std::string& value) {
    std::set<std::string> placeholders;
    size_t start = 0;
    while ((start = value.find('{', start)) != std::string::npos) {
        const size_t end = value.find('}', start + 1);
        if (end == std::string::npos)
            break;
        placeholders.insert(value.substr(start, end - start + 1));
        start = end + 1;
    }
    return placeholders;
}

std::string ReaderValue(IXmlReader* reader) {
    const wchar_t* value = nullptr;
    unsigned int length = 0;
    if (FAILED(reader->GetValue(&value, &length)))
        return {};
    return WideToUtf8(value, length);
}

std::string Attribute(IXmlReader* reader, const wchar_t* name) {
    if (FAILED(reader->MoveToAttributeByName(name, nullptr)))
        return {};
    const std::string value = ReaderValue(reader);
    reader->MoveToElement();
    return value;
}
}

bool LanguageManager::ParseLanguageFile(const std::filesystem::path& path, LanguagePack& pack) {
    IStream* stream = nullptr;
    HRESULT result = SHCreateStreamOnFileEx(path.c_str(), STGM_READ | STGM_SHARE_DENY_WRITE,
                                            FILE_ATTRIBUTE_NORMAL, FALSE, nullptr, &stream);
    if (FAILED(result)) {
        LOG(Warning, "[Language] Failed to open {} (0x{:08X})", path.string(), static_cast<unsigned int>(result));
        return false;
    }

    IXmlReader* reader = nullptr;
    result = CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&reader), nullptr);
    if (FAILED(result)) {
        stream->Release();
        LOG(Warning, "[Language] Failed to create XmlLite reader (0x{:08X})", static_cast<unsigned int>(result));
        return false;
    }

    result = reader->SetInput(stream);
    bool rootSeen = false;
    bool insideText = false;
    std::string currentKey;
    std::string currentValue;
    XmlNodeType nodeType = XmlNodeType_None;

    while (SUCCEEDED(result) && (result = reader->Read(&nodeType)) == S_OK) {
        const wchar_t* localName = nullptr;
        unsigned int localNameLength = 0;
        reader->GetLocalName(&localName, &localNameLength);
        const std::string name = WideToUtf8(localName, localNameLength);

        if (nodeType == XmlNodeType_Element) {
            unsigned int depth = 0;
            reader->GetDepth(&depth);
            if (!rootSeen && depth == 0 && name == "language") {
                rootSeen = true;
                pack.Info.Code = Trim(Attribute(reader, L"code"));
                pack.Info.Name = Trim(Attribute(reader, L"name"));
            }
            else if (rootSeen && depth == 1 && name == "text" && !insideText) {
                currentKey = Trim(Attribute(reader, L"key"));
                currentValue.clear();
                const BOOL isEmpty = reader->IsEmptyElement();
                if (isEmpty) {
                    LOG(Warning, "[Language] Ignoring empty key or value in {}", path.string());
                }
                else {
                    insideText = true;
                }
            }
            else if (insideText) {
                LOG(Warning, "[Language] Nested elements are not supported in {}", path.string());
                result = E_FAIL;
            }
        }
        else if (insideText && (nodeType == XmlNodeType_Text || nodeType == XmlNodeType_CDATA ||
                                nodeType == XmlNodeType_Whitespace)) {
            currentValue += ReaderValue(reader);
        }
        else if (insideText && nodeType == XmlNodeType_EndElement && name == "text") {
            currentValue = Trim(currentValue);
            if (currentKey.empty() || currentValue.empty()) {
                LOG(Warning, "[Language] Ignoring empty key or value in {}", path.string());
            }
            else {
                if (pack.Strings.find(currentKey) != pack.Strings.end())
                    LOG(Warning, "[Language] Duplicate key '{}' in {}; last value wins", currentKey, path.string());
                pack.Strings[currentKey] = currentValue;
            }
            insideText = false;
        }
    }

    reader->Release();
    stream->Release();

    if (FAILED(result) || !rootSeen || pack.Info.Code.empty() || pack.Info.Name.empty()) {
        LOG(Warning, "[Language] Invalid language pack {}", path.string());
        return false;
    }

    pack.Info.FilePath = path;
    return true;
}

bool LanguageManager::Reload(const std::filesystem::path& languageDirectory, const std::string& preferredCode) {
    packs.clear();
    languageInfos.clear();

    LanguagePack english;
    english.Info = { "en-US", "English", {} };
    english.Strings = sEnglishStrings;
    packs.push_back(std::move(english));

    std::error_code error;
    std::vector<std::filesystem::path> files;
    if (std::filesystem::is_directory(languageDirectory, error)) {
        for (const auto& entry : std::filesystem::directory_iterator(languageDirectory, error)) {
            if (error)
                break;
            if (!entry.is_regular_file())
                continue;
            std::string extension = entry.path().extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (extension == ".xml")
                files.push_back(entry.path());
        }
    }
    else if (error) {
        LOG(Warning, "[Language] Cannot scan {}: {}", languageDirectory.string(), error.message());
    }

    std::sort(files.begin(), files.end(), [](const auto& left, const auto& right) {
        return left.filename().wstring() < right.filename().wstring();
    });

    for (const auto& file : files) {
        LanguagePack parsed;
        if (!ParseLanguageFile(file, parsed))
            continue;

        auto duplicate = std::find_if(packs.begin(), packs.end(), [&](const auto& existing) {
            return existing.Info.Code == parsed.Info.Code;
        });
        if (duplicate != packs.end()) {
            if (parsed.Info.Code == "en-US" && duplicate->Info.FilePath.empty()) {
                duplicate->Info.Name = parsed.Info.Name;
                duplicate->Info.FilePath = parsed.Info.FilePath;
                for (const auto& [key, value] : parsed.Strings)
                    duplicate->Strings[key] = value;
            }
            else {
                LOG(Warning, "[Language] Duplicate language code '{}' in {}; first file wins",
                    parsed.Info.Code, file.string());
            }
            continue;
        }
        packs.push_back(std::move(parsed));
    }

    for (const auto& pack : packs)
        languageInfos.push_back(pack.Info);

    if (!Select(preferredCode)) {
        Select("en-US");
        LOG(Warning, "[Language] Language '{}' is unavailable; using en-US", preferredCode);
        return false;
    }
    return true;
}

bool LanguageManager::Select(const std::string& code) {
    const auto found = std::find_if(packs.begin(), packs.end(), [&](const auto& pack) {
        return pack.Info.Code == code;
    });
    if (found == packs.end())
        return false;
    Activate(*found);
    return true;
}

void LanguageManager::Activate(const LanguagePack& pack) {
    activeStrings = sEnglishStrings;
    for (const auto& [key, value] : pack.Strings) {
        const auto fallback = sEnglishStrings.find(key);
        if (fallback != sEnglishStrings.end() && Placeholders(fallback->second) != Placeholders(value)) {
            LOG(Warning, "[Language] Placeholder mismatch for '{}' in {}; using English fallback",
                key, pack.Info.FilePath.string());
            continue;
        }
        activeStrings[key] = value;
    }
    activeCode = pack.Info.Code;
    LOG(Info, "[Language] Active language: {}", activeCode);
}

std::string LanguageManager::Text(const std::string& key) const {
    const auto found = activeStrings.find(key);
    if (found != activeStrings.end())
        return found->second;
    const auto fallback = sEnglishStrings.find(key);
    if (fallback != sEnglishStrings.end())
        return fallback->second;
    LOG(Warning, "[Language] Unknown translation key '{}'", key);
    return key;
}

std::string LanguageManager::Format(
    const std::string& key,
    std::initializer_list<std::pair<std::string, std::string>> replacements) const {
    std::string result = Text(key);
    for (const auto& [name, value] : replacements) {
        const std::string token = "{" + name + "}";
        size_t position = 0;
        while ((position = result.find(token, position)) != std::string::npos) {
            result.replace(position, token.size(), value);
            position += value.size();
        }
    }
    return result;
}

const std::vector<LanguageInfo>& LanguageManager::Languages() const {
    return languageInfos;
}

const std::string& LanguageManager::ActiveCode() const {
    return activeCode;
}
