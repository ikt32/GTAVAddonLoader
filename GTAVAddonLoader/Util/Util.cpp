#define NOMINMAX
#include "Util.hpp"
#include "Logger.hpp"

#include <inc/natives.h>
#include <jpegsize.h>

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

void showText(float x, float y, float scale, const char* text, int font, const Color &rgba, bool outline) {
    HUD::SET_TEXT_FONT(font);
    HUD::SET_TEXT_SCALE(scale, scale);
    HUD::SET_TEXT_COLOUR(rgba.R, rgba.G, rgba.B, rgba.A);
    HUD::SET_TEXT_WRAP(0.0, 1.0);
    HUD::SET_TEXT_CENTRE(0);
    if (outline) HUD::SET_TEXT_OUTLINE();
    HUD::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
    HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((char *)(text));
    HUD::END_TEXT_COMMAND_DISPLAY_TEXT(x, y, 0);
}

void showNotification(std::string message, int *prevNotification) {
    if (prevNotification != nullptr && *prevNotification != 0) {
        HUD::THEFEED_REMOVE_ITEM(*prevNotification);
    }
    HUD::BEGIN_TEXT_COMMAND_THEFEED_POST("STRING");

    HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((char *)(message.c_str()));
    
    int id = HUD::END_TEXT_COMMAND_THEFEED_POST_TICKER(false, false);
    if (prevNotification != nullptr) {
        *prevNotification = id;
    }
}

void showSubtitle(std::string message, int duration) {
    HUD::BEGIN_TEXT_COMMAND_PRINT("CELL_EMAIL_BCON");

    const int maxStringLength = 99;

    for (int i = 0; i < message.size(); i += maxStringLength) {
        int npos = std::min(maxStringLength, static_cast<int>(message.size()) - i);
        HUD::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((char *)(message.substr(i, npos).c_str()));
    }

    HUD::END_TEXT_COMMAND_PRINT(duration, 1);
}

std::optional<std::pair<uint32_t, uint32_t>> GetIMGDimensions(const std::string& path) {
    std::string ext = to_lower(fs::path(path).extension().string());

    std::optional<std::pair<uint32_t, uint32_t>> result;

    if (ext == ".png")
        result = GetPNGDimensions(path);
    if (ext == ".jpg" || ext == ".jpeg")
        result = GetJPGDimensions(path);

    if (result)
        return result;

    logger.Write(ERROR, "[IMG] %s: getting size failed, using defaults (480 x 270)", fs::path(path).filename().string());
    return { {480, 270} };
}

std::optional<std::pair<uint32_t, uint32_t>> GetPNGDimensions(const std::string& path) {
    const uint64_t pngSig = 0x89504E470D0A1A0A;

    std::ifstream img(path, std::ios::binary);

    if (!img.good()) {
        logger.Write(ERROR, "[IMG]: %s failed to open", path.c_str());
        return {};
    }

    uint64_t imgSig = 0x0;

    img.seekg(0);
    img.read((char*)&imgSig, sizeof(uint64_t));

    imgSig = _byteswap_uint64(imgSig);

    if (imgSig == pngSig) {
        uint32_t w, h;

        img.seekg(16);
        img.read((char*)&w, 4);
        img.read((char*)&h, 4);

        w = _byteswap_ulong(w);
        h = _byteswap_ulong(h);

        return { {w, h} };
    }

    logger.Write(ERROR, "[IMG]: %s not a PNG file, sig: 0x%16X", path.c_str(), imgSig);
    return {};
}

std::optional<std::pair<uint32_t, uint32_t>> GetJPGDimensions(const std::string& path) {
    FILE *image = nullptr;

    errno_t err = fopen_s(&image, path.c_str(), "rb");  // open JPEG image file
    if (!image || err) {
        logger.Write(ERROR, "JPEG error: %s: Failed to open file", fs::path(path).filename().string().c_str());
        if (image)
            fclose(image);
        return {};
    }
    int w, h;
    int result = scanhead(image, &w, &h);
    fclose(image);

    if (result == 1) {
        return { {w, h} };
    }
    else {
        return {};
    }
}

Hash joaat(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);

    Hash hash = 0;
    for (int i = 0; i < s.size(); i++) {
        hash += s[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

std::string getGxtName(Hash hash) {
    const char *name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(hash);
    std::string displayName = HUD::_GET_LABEL_TEXT(name);
    if (displayName == "NULL") {
        displayName = name;
    }
    return displayName;
}

std::string to_lower(std::string data) {
    std::transform(data.begin(), data.end(), data.begin(), ::tolower);
    return data;
}

bool FileExists(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

std::string removeSpecialChars(std::string input) {
    input.erase(std::remove(input.begin(), input.end(), ' '), input.end());
    input.erase(std::remove(input.begin(), input.end(), '-'), input.end());
    return input;
}
