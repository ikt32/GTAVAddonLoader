#pragma once
#include <set>
#include <tuple>
#include <string>

#include <inc/types.h>

// filePath, width, height
using AddonImageMeta = std::tuple<std::string, int, int>;

// className, vehicleHash
//using AddonVehicle = std::pair<std::string, Hash>;

class ModelInfo {
public:
    ModelInfo(): ModelHash(0) { }

    ModelInfo(std::string className, std::string makeName, Hash hash) :
              ClassName(className), MakeName(makeName), ModelHash(hash) { }

    std::string ClassName;
    std::string MakeName;
    Hash ModelHash;
};

class AddonImage {
public:
    AddonImage() : ModelHash(0), ResX(0), ResY(0), TextureID(0) {}
    AddonImage(int textureId, Hash hash, uint16_t resX, uint16_t resY) :
        ModelHash(hash),
        ResX(resX),
        ResY(resY),
        TextureID(textureId) { }

    Hash ModelHash;
    uint16_t ResX;
    uint16_t ResY;
    int TextureID;
};

class SpriteInfo {
public:
    SpriteInfo() : ModelHash(0), ResX(0), ResY(0) {}
    SpriteInfo(std::string dict, std::string name, Hash hash, uint16_t resX, uint16_t resY) :
        Dict(dict), Name(name), ModelHash(hash), ResX(resX), ResY(resY) { }

    std::string Dict;
    std::string Name;
    Hash ModelHash;
    uint16_t ResX;
    uint16_t ResY;
};

class DLC {
public:
    DLC(std::string name, std::vector<Hash> hashes) :
        Name(name), Hashes(hashes)
    { }
    std::string Name;
    std::set<std::string> Classes;
    std::set<std::string> Makes;
    std::vector<Hash> Hashes;
    //std::vector<std::pair<std::string, Hash>> Vehicles;
    std::vector<ModelInfo> Vehicles;
};
