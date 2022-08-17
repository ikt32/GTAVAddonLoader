#pragma once
#include <inc/types.h>

#include <set>
#include <string>
#include <vector>

class ModelInfo {
public:
    ModelInfo(): ModelHash(0) { }

    ModelInfo(std::string className, std::string makeName, std::string modelName, Hash hash) :
              ClassName(className), MakeName(makeName), ModelName(modelName), ModelHash(hash) { }

    ModelInfo(std::string className, std::string makeName, std::string modelName, Hash hash, std::vector<std::string> notes) :
        ClassName(className), MakeName(makeName), ModelName(modelName), ModelHash(hash), Notes(notes) {
    }

    std::string ClassName;
    std::string MakeName;
    std::string ModelName;
    Hash ModelHash;
    std::vector<std::string> Notes;
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

class DLCDefinition {
public:
    DLCDefinition(std::string name,
                  std::vector<Hash> hashes,
                  std::string note = std::string())
        : Name(name)
        , Hashes(hashes)
        , Note(note)
    { }
    std::string Name;
    std::set<std::string> Classes;
    std::set<std::string> Makes;
    std::vector<Hash> Hashes;
    std::vector<ModelInfo> Vehicles; // TODO: Combine with Hashes

    std::string Note;
};
