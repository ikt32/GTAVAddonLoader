#define NOMINMAX
#include "../../../ScriptHookV_SDK/inc/natives.h"
#include "Util.hpp"
#include <algorithm>
#include <experimental/filesystem>
#include <lodepng/lodepng.h>
#include <jpegsize.h>

#include "Logger.hpp"

namespace fs = std::experimental::filesystem;

void showText(float x, float y, float scale, const char* text, int font, const Color &rgba, bool outline) {
    UI::SET_TEXT_FONT(font);
    UI::SET_TEXT_SCALE(scale, scale);
    UI::SET_TEXT_COLOUR(rgba.R, rgba.G, rgba.B, rgba.A);
    UI::SET_TEXT_WRAP(0.0, 1.0);
    UI::SET_TEXT_CENTRE(0);
    if (outline) UI::SET_TEXT_OUTLINE();
    UI::BEGIN_TEXT_COMMAND_DISPLAY_TEXT("STRING");
    UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((char *)(text));
    UI::END_TEXT_COMMAND_DISPLAY_TEXT(x, y);
}

void showNotification(std::string message, int *prevNotification) {
    if (prevNotification != nullptr && *prevNotification != 0) {
        UI::_REMOVE_NOTIFICATION(*prevNotification);
    }
    UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");

    UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((char *)(message.c_str()));
    
    int id = UI::_DRAW_NOTIFICATION(false, false);
    if (prevNotification != nullptr) {
        *prevNotification = id;
    }
}

void showSubtitle(std::string message, int duration) {
    UI::BEGIN_TEXT_COMMAND_PRINT("CELL_EMAIL_BCON");

    const int maxStringLength = 99;

    for (int i = 0; i < message.size(); i += maxStringLength) {
        int npos = std::min(maxStringLength, static_cast<int>(message.size()) - i);
        UI::ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME((char *)(message.substr(i, npos).c_str()));
    }

    UI::END_TEXT_COMMAND_PRINT(duration, 1);
}

GameSound::GameSound(char *sound, char *soundSet): m_prevNotification(0) {
    Active = false;
    m_sound = sound;
    m_soundSet = soundSet;
    m_soundID = -1;
}

GameSound::~GameSound() {
    if (m_soundID == -1 || !Active) return;
    AUDIO::RELEASE_SOUND_ID(m_soundID);
}

void GameSound::Load(char *audioBank) {
    AUDIO::REQUEST_SCRIPT_AUDIO_BANK(audioBank, false);
}

void GameSound::Play(Entity ent) {
    if (Active) return;
    m_soundID = AUDIO::GET_SOUND_ID();
    //showNotification(("New soundID: " + std::to_string(m_soundID)).c_str(), nullptr);
    AUDIO::PLAY_SOUND_FROM_ENTITY(m_soundID, m_sound, ent, m_soundSet, 0, 0);
    Active = true;
}

void GameSound::Stop() {
    if (m_soundID == -1 || !Active) return;
    AUDIO::STOP_SOUND(m_soundID);
    Active = false;
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
    char *name = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(hash);
    std::string displayName = UI::_GET_LABEL_TEXT(name);
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
