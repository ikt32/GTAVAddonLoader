#define NOMINMAX
#include "inc/natives.h"
#include "Util.hpp"
#include <algorithm>
#include <filesystem>
#include <lodepng/lodepng.h>
#include <jpegsize.h>

#include "Logger.hpp"

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

bool GetIMGDimensions(std::string file, unsigned *width, unsigned *height) {
    auto ext = fs::path(file).extension();
    if (ext == ".png" || ext == ".PNG")
        return GetPNGDimensions(file, width, height);
    if (ext == ".jpg" || ext == ".JPG" || ext == ".jpeg" || ext == ".JPEG")
        return GetJPGDimensions(file, width, height);
    return false;
}

bool GetPNGDimensions(std::string file, unsigned *width, unsigned *height) {
    std::vector<unsigned char> image;
    unsigned error = lodepng::decode(image, *width, *height, file);
    if (error) {
        logger.Write(ERROR, "PNG error: %s: %s", std::to_string(error).c_str(), lodepng_error_text(error));
        return false;
    }
    return true;
}

bool GetJPGDimensions(std::string file, unsigned *width, unsigned *height) {
    FILE *image = nullptr;

    errno_t err = fopen_s(&image, file.c_str(), "rb");  // open JPEG image file
    if (!image || err) {
        logger.Write(ERROR, "JPEG error: %s: Failed to open file", fs::path(file).filename().string().c_str());
        return false;
    }
    int w, h;
    int result = scanhead(image, &w, &h);
    if (result == 1) {
        *width = w;
        *height = h;
        return true;
    }
    logger.Write(ERROR, "JPEG error: %s: getting size failed, using defaults (480 x 270)", fs::path(file).filename().string());
    *width = 480;
    *height = 270;
    return false;
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
