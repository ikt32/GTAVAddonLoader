#pragma once

#include <inc/main.h>

#include <format>
#include <string>
#include <vector>


namespace Versions {
const std::vector<std::string> NamesLegacy = {
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

    "v1.0.2699.16",      // 79
    "v1.0.2802.0",       // 80
    "v1.0.2824.0",       // 81
    "v1.0.2845.0",       // 82
    "v1.0.2944.0",       // 83
    "v1.0.3028.0",       // 84
    "v1.0.3095.0",       // 85
    "v1.0.3179.0",       // 86
    "v1.0.3258.0",       // 87
    "v1.0.3274.0",       // 88
    "v1.0.3323.0",       // 89
    "v1.0.3337.0",       // 90
    "v1.0.3351.0",       // 91
    "v1.0.3407.0",       // 92
    "v1.0.3411.0",       // 93
    "v1.0.3442.0",       // 94
    "v1.0.3504.0",       // 95
    "v1.0.3521.0",       // 96
    "v1.0.3570.0",       // 97
    "v1.0.3586.0",       // 98
    "v1.0.3717.0",       // 99
    //"v1.0.3725.0",     // XXX
    "v1.0.3751.0",       // 100
    "v1.0.3788.0",       // 101
};

const std::vector<std::string> NamesEnhanced{
    "Invalid",          // 1000
    "v1.0.811.8",       // 1001
    "v1.0.812.8",       // 1002
    "v1.0.813.11",      // 1003
    "v1.0.814.9",       // 1004
    "v1.0.889.15",      // 1005
    "v1.0.889.19",      // 1006
    "v1.0.889.22",      // 1007
    "v1.0.1013.17",     // 1008
    //"v1.0.1013.20"    // XXXX
    "v1.0.1013.29",     // 1009
    "v1.0.1013.33",     // 1010
};

enum EGameVersion: int {
    L_1_0_335_2_STEAM,      // 00
    L_1_0_335_2_NOSTEAM,    // 01

    L_1_0_350_1_STEAM,      // 02
    L_1_0_350_2_NOSTEAM,    // 03

    L_1_0_372_2_STEAM,      // 04
    L_1_0_372_2_NOSTEAM,    // 05

    L_1_0_393_2_STEAM,      // 06
    L_1_0_393_2_NOSTEAM,    // 07

    L_1_0_393_4_STEAM,      // 08
    L_1_0_393_4_NOSTEAM,    // 09

    L_1_0_463_1_STEAM,      // 10
    L_1_0_463_1_NOSTEAM,    // 11

    L_1_0_505_2_STEAM,      // 12
    L_1_0_505_2_NOSTEAM,    // 13

    L_1_0_573_1_STEAM,      // 14
    L_1_0_573_1_NOSTEAM,    // 15

    L_1_0_617_1_STEAM,      // 16
    L_1_0_617_1_NOSTEAM,    // 17

    L_1_0_678_1_STEAM,      // 18
    L_1_0_678_1_NOSTEAM,    // 19

    L_1_0_757_2_STEAM,      // 20
    L_1_0_757_2_NOSTEAM,    // 21

    L_1_0_757_4_STEAM,      // 22
    L_1_0_757_4_NOSTEAM,    // 23

    L_1_0_791_2_STEAM,      // 24
    L_1_0_791_2_NOSTEAM,    // 25

    L_1_0_877_1_STEAM,      // 26
    L_1_0_877_1_NOSTEAM,    // 27

    L_1_0_944_2_STEAM,      // 28
    L_1_0_944_2_NOSTEAM,    // 29

    L_1_0_1011_1_STEAM,     // 30
    L_1_0_1011_1_NOSTEAM,   // 31

    L_1_0_1032_1_STEAM,     // 32
    L_1_0_1032_1_NOSTEAM,   // 33

    L_1_0_1103_2_STEAM,     // 34
    L_1_0_1103_2_NOSTEAM,   // 35

    L_1_0_1180_2_STEAM,     // 36
    L_1_0_1180_2_NOSTEAM,   // 37

    L_1_0_1290_1_STEAM,     // 38
    L_1_0_1290_1_NOSTEAM,   // 39

    L_1_0_1365_1_STEAM,     // 40
    L_1_0_1365_1_NOSTEAM,   // 41

    L_1_0_1493_0_STEAM,     // 42
    L_1_0_1493_0_NOSTEAM,   // 43

    L_1_0_1493_1_STEAM,     // 44
    L_1_0_1493_1_NOSTEAM,   // 45

    L_1_0_1604_0_STEAM,     // 46
    L_1_0_1604_0_NOSTEAM,   // 47

    L_1_0_1604_1_STEAM,     // 48
    L_1_0_1604_1_NOSTEAM,   // 49

    //L_1_0_1734_0_STEAM,   // XX
    //L_1_0_1734_0_NOSTEAM, // XX

    L_1_0_1737_0_STEAM,     // 50
    L_1_0_1737_0_NOSTEAM,   // 51

    L_1_0_1737_6_STEAM,     // 52
    L_1_0_1737_6_NOSTEAM,   // 53

    L_1_0_1868_0_STEAM,     // 54
    L_1_0_1868_0_NOSTEAM,   // 55

    L_1_0_1868_1_STEAM,     // 56
    L_1_0_1868_1_NOSTEAM,   // 57

    L_1_0_1868_4_EGS,       // 58

    L_1_0_2060_0_STEAM,     // 59
    L_1_0_2060_0_NOSTEAM,   // 60

    L_1_0_2060_1_STEAM,     // 61
    L_1_0_2060_1_NOSTEAM,   // 62

    L_1_0_2189_0_STEAM,     // 63
    L_1_0_2189_0_NOSTEAM,   // 64

    L_1_0_2215_0_STEAM,     // 65
    L_1_0_2215_0_NOSTEAM,   // 66

    L_1_0_2245_0_STEAM,     // 67
    L_1_0_2245_0_NOSTEAM,   // 68

    L_1_0_2372_0_STEAM,     // 69
    L_1_0_2372_0_NOSTEAM,   // 70

    L_1_0_2545_0_STEAM,     // 71
    L_1_0_2545_0_NOSTEAM,   // 72

    L_1_0_2612_1_STEAM,     // 73
    L_1_0_2612_1_NOSTEAM,   // 74

    L_1_0_2628_2_STEAM,     // 75
    L_1_0_2628_2_NOSTEAM,   // 76

    L_1_0_2699_0_STEAM,     // 77
    L_1_0_2699_0_NOSTEAM,   // 78

    L_1_0_2699_16,          // 79
    L_1_0_2802_0,           // 80
    L_1_0_2824_0,           // 81
    L_1_0_2845_0,           // 82
    L_1_0_2944_0,           // 83
    L_1_0_3028_0,           // 84
    L_1_0_3095_0,           // 85
    L_1_0_3179_0,           // 86
    L_1_0_3258_0,           // 87
    L_1_0_3274_0,           // 88
    L_1_0_3323_0,           // 89
    L_1_0_3337_0,           // 90
    L_1_0_3351_0,           // 91
    L_1_0_3407_0,           // 92
    L_1_0_3411_0,           // 93
    L_1_0_3442_0,           // 94
    L_1_0_3504_0,           // 95
    L_1_0_3521_0,           // 96
    L_1_0_3570_0,           // 97
    L_1_0_3586_0,           // 98
    L_1_0_3717_0,           // 99
    //L_1_0_3725_0          // XXX
    L_1_0_3751_0,           // 100
    L_1_0_3788_0,           // 101

    // Enhanced
    E_1_0_811_8             = 1001,
    E_1_0_812_8             = 1002,
    E_1_0_813_11            = 1003,
    E_1_0_814_9             = 1004,
    E_1_0_889_15            = 1005,
    E_1_0_889_19            = 1006,
    E_1_0_889_22            = 1006,
    E_1_0_1013_17           = 1008,
    //E_1_0_1013_20         = XXXX
    E_1_0_1013_29           = 1009,
    E_1_0_1013_33           = 1010,
};

inline bool IsEnhanced(int version) {
    return version >= E_1_0_811_8;
}

inline bool IsEnhanced() {
    return IsEnhanced(getGameVersion());
}

inline std::string GetNameEnhanced(int version) {
    version = version - 1000;
    if (version > NamesEnhanced.size() - 1 || version < 0) {
        return std::format("Enhanced - Unknown {} (Names: {})", version, NamesEnhanced.size());
    }
    return std::format("Enhanced - {}", NamesEnhanced[version]);
}

inline std::string GetNameLegacy(int version) {
    if (version > NamesLegacy.size() - 1 || version < 0) {
        return std::format("Legacy - Unknown {} (Names: {})", version, NamesLegacy.size());
    }
    return std::format("Legacy - {}", NamesLegacy[version]);
}

inline std::string GetName(int version) {
    if (IsEnhanced()) {
        return GetNameEnhanced(version);
    }
    return GetNameLegacy(version);
}
}
