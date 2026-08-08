#pragma once
#include <inc/types.h>
#include <string>
#include <vector>

struct VehicleModData {
    struct VehicleMod {
        enum class ModType {
            Color,
//            ColorCombo,
            ColorCustom1,
            ColorCustom2,
            ExtraColors,
            Mod,
            ModToggle,
            WheelType,
            WindowTint,
            TyresCanBurst,
            Livery,
            TyreSmoke,
            ModColor1,
            ModColor2,
            Extra,
            Neon,
            NeonColor,
            XenonColor,
            PlateType,
            InteriorColor,
            DashboardColor,
        };

        Vehicle Handle;
        ModType Type;
        int ModId = -1;

        int Value0;
        int Value1;
        int Value2;
        int Value3;

        VehicleMod Get();

        void Set();
    };

    Vehicle Handle;

    std::string Plate;


    std::vector<VehicleMod> VehicleMods;

    void GetAll();

    void SetAll();

    // Sets all performance mods (engine, brakes, transmission, suspension,
    // armor, turbo) to their maximum level. Intended for use right after
    // spawning a vehicle.
    static void SetMaxPerformanceMods(Vehicle vehicle);
};
