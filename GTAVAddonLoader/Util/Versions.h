#pragma once
#include <string>
#include <vector>

static std::vector<std::string> GameVersionString = {
    "v1.0.335.2.STEAM",      // 00
    "v1.0.335.2.NOSTEAM",    // 01

    "v1.0.350.1.STEAM",      // 02
    "v1.0.350.2.NOSTEAM",    // 03

    "v1.0.372.2.STEAM",      // 04
    "v1.0.372.2.NOSTEAM",    // 05

    "v1.0.393.2.STEAM",      // 06
    "v1.0.393.2.NOSTEAM",    // 07

    "v1.0.393.4.STEAM",      // 08
    "v1.0.393.4.NOSTEAM",    // 09

    "v1.0.463.1.STEAM",      // 10
    "v1.0.463.1.NOSTEAM",    // 11

    "v1.0.505.2.STEAM",      // 12
    "v1.0.505.2.NOSTEAM",    // 13

    "v1.0.573.1.STEAM",      // 14
    "v1.0.573.1.NOSTEAM",    // 15

    "v1.0.617.1.STEAM",      // 16
    "v1.0.617.1.NOSTEAM",    // 17

    "v1.0.678.1.STEAM",      // 18
    "v1.0.678.1.NOSTEAM",    // 19

    "v1.0.757.2.STEAM",      // 20
    "v1.0.757.2.NOSTEAM",    // 21

    "v1.0.757.4.STEAM",      // 22
    "v1.0.757.4.NOSTEAM",    // 23

    "v1.0.791.2.STEAM",      // 24
    "v1.0.791.2.NOSTEAM",    // 25

    "v1.0.877.1.STEAM",      // 26
    "v1.0.877.1.NOSTEAM",    // 27

    "v1.0.944.2.STEAM",      // 28
    "v1.0.944.2.NOSTEAM",    // 29

    "v1.0.1011.1.STEAM",     // 30
    "v1.0.1011.1.NOSTEAM",   // 31

    "v1.0.1032.1.STEAM",     // 32
    "v1.0.1032.1.NOSTEAM",   // 33

    "v1.0.1103.2.STEAM",     // 34
    "v1.0.1103.2.NOSTEAM",   // 35

    "v1.0.1180.2.STEAM",     // 36
    "v1.0.1180.2.NOSTEAM",   // 37

    "v1.0.1290.1.STEAM",     // 38
    "v1.0.1290.1.NOSTEAM",   // 39

    "v1.0.1365.1.STEAM",     // 40
    "v1.0.1365.1.NOSTEAM",   // 41

    "v1.0.1493.0.STEAM",     // 42
    "v1.0.1493.0.NOSTEAM",   // 43

    "v1.0.1493.1.STEAM",     // 44
    "v1.0.1493.1.NOSTEAM",   // 45

    "v1.0.1604.0.STEAM",     // 46
    "v1.0.1604.0.NOSTEAM",   // 47

    "v1.0.1604.1.STEAM",     // 48
    "v1.0.1604.1.NOSTEAM",   // 49

    //"v1.0.1734.0.STEAM",   // XX
    //"v1.0.1734.0.NOSTEAM", // XX

    "v1.0.1737.0.STEAM",     // 50
    "v1.0.1737.0.NOSTEAM",   // 51

    "v1.0.1737.6.STEAM",     // 52
    "v1.0.1737.6.NOSTEAM",   // 53

    "v1.0.1868.0.STEAM",     // 54
    "v1.0.1868.0.NOSTEAM",   // 55

    "v1.0.1868.1.STEAM",     // 56
    "v1.0.1868.1.NOSTEAM",   // 57

    "v1.0.1868.4.EGS",       // 58

    "v1.0.2060.0.STEAM",     // 59
    "v1.0.2060.0.NOSTEAM",   // 60

    "v1.0.2060.1.STEAM",     // 61
    "v1.0.2060.1.NOSTEAM",   // 62

    "v1.0.2189.0.STEAM",     // 63
    "v1.0.2189.0.NOSTEAM",   // 64

    "v1.0.2215.0.STEAM",     // 65
    "v1.0.2215.0.NOSTEAM",   // 66

    "v1.0.2245.0.STEAM",     // 67
    "v1.0.2245.0.NOSTEAM",   // 68

    "v1.0.2372.0.STEAM",     // 69
    "v1.0.2372.0.NOSTEAM",   // 70

    "v1.0.2545.0.STEAM",     // 71
    "v1.0.2545.0.NOSTEAM",   // 72

    "v1.0.2612.1.STEAM",     // 73
    "v1.0.2612.1.NOSTEAM",   // 74

    "v1.0.2628.2.STEAM",     // 75
    "v1.0.2628.2.NOSTEAM",   // 76

    "v1.0.2699.0.STEAM",     // 77
    "v1.0.2699.0.NOSTEAM",   // 78

    "v1.0.2699.16",          // 79
    "v1.0.2802.0",           // 80
    "v1.0.2824.0",           // 81
    "v1.0.2845.0",           // 82
    "v1.0.2944.0",           // 83
    "v1.0.3028.0",           // 84
    "v1.0.3095.0",           // 85
    "v1.0.3179.0",           // 86
    "v1.0.3258.0",           // 87
    "v1.0.3274.0",           // 88
};

static std::string eGameVersionToString(int version) {
    if (version > GameVersionString.size() - 1 || version < 0) {
        return std::to_string(version);
    }
    return GameVersionString[version];
}
