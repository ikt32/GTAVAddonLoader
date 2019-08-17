#pragma once
#include <inc/types.h>

#include <set>
#include <string>
#include <vector>

class ModelInfo {
public:
    ModelInfo(): ModelHash(0) { }

    ModelInfo(std::string className, std::string makeName, Hash hash) :
              ClassName(className), MakeName(makeName), ModelHash(hash) { }

    std::string ClassName;
    std::string MakeName;
    Hash ModelHash;
    // TODO: AddonImage?
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

class DLC {
public:
    DLC(std::string name, std::vector<Hash> hashes) :
        Name(name), Hashes(hashes)
    { }
    std::string Name;
    std::set<std::string> Classes;
    std::set<std::string> Makes;
    std::vector<Hash> Hashes;
    std::vector<ModelInfo> Vehicles; // TODO: Combine with Hashes
};
