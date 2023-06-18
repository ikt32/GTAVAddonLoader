#pragma once
#include <string>
#include <vector>

static std::vector<std::string> GameVersionString = {
    "VER_1_0_335_2_STEAM",      // 00
    "VER_1_0_335_2_NOSTEAM",    // 01

    "VER_1_0_350_1_STEAM",      // 02
    "VER_1_0_350_2_NOSTEAM",    // 03

    "VER_1_0_372_2_STEAM",      // 04
    "VER_1_0_372_2_NOSTEAM",    // 05

    "VER_1_0_393_2_STEAM",      // 06
    "VER_1_0_393_2_NOSTEAM",    // 07

    "VER_1_0_393_4_STEAM",      // 08
    "VER_1_0_393_4_NOSTEAM",    // 09

    "VER_1_0_463_1_STEAM",      // 10
    "VER_1_0_463_1_NOSTEAM",    // 11

    "VER_1_0_505_2_STEAM",      // 12
    "VER_1_0_505_2_NOSTEAM",    // 13

    "VER_1_0_573_1_STEAM",      // 14
    "VER_1_0_573_1_NOSTEAM",    // 15

    "VER_1_0_617_1_STEAM",      // 16
    "VER_1_0_617_1_NOSTEAM",    // 17

    "VER_1_0_678_1_STEAM",      // 18
    "VER_1_0_678_1_NOSTEAM",    // 19

    "VER_1_0_757_2_STEAM",      // 20
    "VER_1_0_757_2_NOSTEAM",    // 21

    "VER_1_0_757_4_STEAM",      // 22
    "VER_1_0_757_4_NOSTEAM",    // 23

    "VER_1_0_791_2_STEAM",      // 24
    "VER_1_0_791_2_NOSTEAM",    // 25

    "VER_1_0_877_1_STEAM",      // 26
    "VER_1_0_877_1_NOSTEAM",    // 27

    "VER_1_0_944_2_STEAM",      // 28
    "VER_1_0_944_2_NOSTEAM",    // 29

    "VER_1_0_1011_1_STEAM",     // 30
    "VER_1_0_1011_1_NOSTEAM",   // 31

    "VER_1_0_1032_1_STEAM",     // 32
    "VER_1_0_1032_1_NOSTEAM",   // 33

    "VER_1_0_1103_2_STEAM",     // 34
    "VER_1_0_1103_2_NOSTEAM",   // 35

    "VER_1_0_1180_2_STEAM",     // 36
    "VER_1_0_1180_2_NOSTEAM",   // 37

    "VER_1_0_1290_1_STEAM",     // 38
    "VER_1_0_1290_1_NOSTEAM",   // 39

    "VER_1_0_1365_1_STEAM",     // 40
    "VER_1_0_1365_1_NOSTEAM",   // 41

    "VER_1_0_1493_0_STEAM",     // 42
    "VER_1_0_1493_0_NOSTEAM",   // 43

    "VER_1_0_1493_1_STEAM",     // 44
    "VER_1_0_1493_1_NOSTEAM",   // 45

    "VER_1_0_1604_0_STEAM",     // 46
    "VER_1_0_1604_0_NOSTEAM",   // 47

    "VER_1_0_1604_1_STEAM",     // 48
    "VER_1_0_1604_1_NOSTEAM",   // 49

    //"VER_1_0_1734_0_STEAM",   // XX
    //"VER_1_0_1734_0_NOSTEAM", // XX

    "VER_1_0_1737_0_STEAM",     // 50
    "VER_1_0_1737_0_NOSTEAM",   // 51

    "VER_1_0_1737_6_STEAM",     // 52
    "VER_1_0_1737_6_NOSTEAM",   // 53

    "VER_1_0_1868_0_STEAM",     // 54
    "VER_1_0_1868_0_NOSTEAM",   // 55

    "VER_1_0_1868_1_STEAM",     // 56
    "VER_1_0_1868_1_NOSTEAM",   // 57

    "VER_1_0_1868_4_EGS",       // 58

    "VER_1_0_2060_0_STEAM",     // 59
    "VER_1_0_2060_0_NOSTEAM",   // 60
    //"VER_1_0_2060_0_EGS",     // XX

    "VER_1_0_2060_1_STEAM",     // 61
    "VER_1_0_2060_1_NOSTEAM",   // 62

    "VER_1_0_2189_0_STEAM",     // 63
    "VER_1_0_2189_0_NOSTEAM",   // 64

    "VER_1_0_2215_0_STEAM",     // 65
    "VER_1_0_2215_0_NOSTEAM",   // 66

    "VER_1_0_2245_0_STEAM",     // 67
    "VER_1_0_2245_0_NOSTEAM",   // 68

    "VER_1_0_2372_0_STEAM",     // 69
    "VER_1_0_2372_0_NOSTEAM",   // 70

    "VER_1_0_2545_0_STEAM",     // 71
    "VER_1_0_2545_0_NOSTEAM",   // 72

    "VER_1_0_2612_1_STEAM",     // 73
    "VER_1_0_2612_1_NOSTEAM",   // 74

    "VER_1_0_2628_2_STEAM",     // 75
    "VER_1_0_2628_2_NOSTEAM",   // 76

    "VER_1_0_2699_0_STEAM",     // 77
    "VER_1_0_2699_0_NOSTEAM",   // 78

    "VER_1_0_2699_16",          // 79
    "VER_1_0_2802_0",           // 80
    "VER_1_0_2824_0",           // 81
    "VER_1_0_2845_0",           // 82
    "VER_1_0_2944_0",           // 83
};

static std::string eGameVersionToString(int version) {
    if (version > GameVersionString.size() - 1 || version < 0) {
        return std::to_string(version);
    }
    return GameVersionString[version];
}
