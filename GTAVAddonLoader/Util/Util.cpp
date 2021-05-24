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
    if (ext == ".webp")
        result = GetWebPDimensions(path);

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
        logger.Write(ERROR, "[IMG]: %s: Failed to open file", fs::path(path).filename().string().c_str());
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

std::optional<std::pair<uint32_t, uint32_t>> GetWebPDimensions(const std::string& path) {
    const uint32_t riffSig = 'RIFF';
    const uint32_t webpSig = 'WEBP';

    std::ifstream img(path, std::ios::binary);

    if (!img.good()) {
        logger.Write(ERROR, "[IMG]: %s failed to open", path.c_str());
        return {};
    }

    uint32_t imgRiffSig = 0x0;
    uint32_t imgWebPSig = 0x0;

    img.seekg(0);
    img.read((char*)&imgRiffSig, sizeof(uint32_t));
    imgRiffSig = _byteswap_ulong(imgRiffSig);

    img.seekg(2 * sizeof(uint32_t));
    img.read((char*)&imgWebPSig, sizeof(uint32_t));
    imgWebPSig = _byteswap_ulong(imgWebPSig);

    if (imgRiffSig == riffSig && imgWebPSig == webpSig) {
        uint32_t vp8Sig = 0x0;

        img.seekg(3 * sizeof(uint32_t));
        img.read((char*)&vp8Sig, sizeof(uint32_t));
        vp8Sig = _byteswap_ulong(vp8Sig);

        switch (vp8Sig) {
            case 'VP8 ': {
                uint8_t sigBytes[3] = { 0x0, 0x0, 0x0 };
                img.seekg(0x17);
                img.read((char*)sigBytes, 3);
                if (sigBytes[0] != 0x9D || sigBytes[1] != 0x01 || sigBytes[2] != 0x2A) {
                    logger.Write(ERROR, "[IMG]: %s failed to find VP8 (Lossy) signature bytes, got 0x%02X 0x%02X 0x%02X",
                        path.c_str(), sigBytes[0], sigBytes[1], sigBytes[2]);
                    return {};
                }

                uint16_t w = 0x0;
                uint16_t h = 0x0;

                img.read((char*)&w, 2);
                img.read((char*)&h, 2);

                return { {w, h} };
            }
            case 'VP8L': {
                uint8_t sigByte = 0x0;
                img.seekg(5 * sizeof(uint32_t));
                img.read((char*)&sigByte, 1);
                if (sigByte != 0x2F) {
                    logger.Write(ERROR, "[IMG]: %s failed to find VP8 (Lossless) signature byte, got 0x%02X",
                        path.c_str(), sigByte);
                    return {};
                }

                uint32_t wh = 0x0;

                img.read((char*)&wh, 4);
                wh = wh & 0x0FFFFFFF;

                uint16_t w = wh & 0x3FFF;
                uint16_t h = (wh >> 14) & 0x3FFF;
                return { {w + 1, h + 1} };
            }
            default:
                logger.Write(ERROR, "[IMG]: %s unrecognized WebP format. FourCC: 0x%04X",
                    path.c_str(), vp8Sig);
                return {};
        }
    }
    else {
        logger.Write(ERROR, "[IMG]: %s not a WebP file. RIFF (0x%04X): 0x%04X, WEBP (0x%04X): 0x%04X",
            path.c_str(), riffSig, imgRiffSig, webpSig, imgWebPSig);
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
