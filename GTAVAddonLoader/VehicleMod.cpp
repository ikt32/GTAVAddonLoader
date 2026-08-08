#include "VehicleMod.h"

#include "Util/Versions.h"

#include <inc/natives.h>

namespace VEHICLE {
    // b877
    void _GET_VEHICLE_INTERIOR_COLOUR(Vehicle vehicle, int* colour) {
        if (getGameVersion() < Versions::EGameVersion::L_1_0_877_1_STEAM)
            return;
        invoke<Void>(0x7D1464D472D32136, vehicle, colour);
    }

    void _SET_VEHICLE_INTERIOR_COLOUR(Vehicle vehicle, int colour) {
        if (getGameVersion() < Versions::EGameVersion::L_1_0_877_1_STEAM)
            return;
        invoke<Void>(0xF40DD601A65F7F19, vehicle, colour);
    }

    void _GET_VEHICLE_DASHBOARD_COLOUR(Vehicle vehicle, int* colour) {
        if (getGameVersion() < Versions::EGameVersion::L_1_0_877_1_STEAM)
            return;
        invoke<Void>(0xB7635E80A5C31BFF, vehicle, colour);
    }

    void _SET_VEHICLE_DASHBOARD_COLOUR(Vehicle vehicle, int colour) {
        if (getGameVersion() < Versions::EGameVersion::L_1_0_877_1_STEAM)
            return;
        invoke<Void>(0x6089CDF6A57F326C, vehicle, colour);
    }

    // b1604
    static void _SET_VEHICLE_XENON_LIGHTS_COLOUR(Vehicle vehicle, int colorIndex) {
        if (getGameVersion() < Versions::EGameVersion::L_1_0_1604_0_STEAM)
            return;
        invoke<Void>(0xE41033B25D003A07, vehicle, colorIndex);
    }

    static int _GET_VEHICLE_XENON_LIGHTS_COLOUR(Vehicle vehicle) {
        if (getGameVersion() < Versions::EGameVersion::L_1_0_1604_0_STEAM)
            return 0;
        return invoke<int>(0x3DFF319A831E0CDB, vehicle);
    }
}

VehicleModData::VehicleMod VehicleModData::VehicleMod::Get() {
    switch (Type) {
        case ModType::Color:
            VEHICLE::GET_VEHICLE_COLOURS(Handle, &Value0, &Value1);
            break;
        case ModType::ExtraColors:
            VEHICLE::GET_VEHICLE_EXTRA_COLOURS(Handle, &Value0, &Value1);
            break;
        case ModType::ColorCustom1:
            Value0 = VEHICLE::GET_IS_VEHICLE_PRIMARY_COLOUR_CUSTOM(Handle);
            VEHICLE::GET_VEHICLE_CUSTOM_PRIMARY_COLOUR(Handle, &Value1, &Value2, &Value3);
            break;
        case ModType::ColorCustom2:
            Value0 = VEHICLE::GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM(Handle);
            VEHICLE::GET_VEHICLE_CUSTOM_SECONDARY_COLOUR(Handle, &Value1, &Value2, &Value3);
            break;
        case ModType::Mod:
            Value0 = VEHICLE::GET_VEHICLE_MOD(Handle, ModId);
            Value1 = VEHICLE::GET_VEHICLE_MOD_VARIATION(Handle, ModId);
            break;
        case ModType::ModToggle:
            Value0 = VEHICLE::IS_TOGGLE_MOD_ON(Handle, ModId);
            break;
        case ModType::WheelType:
            Value0 = VEHICLE::GET_VEHICLE_WHEEL_TYPE(Handle);
            break;
        case ModType::WindowTint:
            Value0 = VEHICLE::GET_VEHICLE_WINDOW_TINT(Handle);
            break;
        case ModType::TyresCanBurst:
            Value0 = VEHICLE::GET_VEHICLE_TYRES_CAN_BURST(Handle);
            break;
        case ModType::TyreSmoke:
            VEHICLE::GET_VEHICLE_TYRE_SMOKE_COLOR(Handle, &Value0, &Value1, &Value2);
            break;
        case ModType::Livery:
            Value0 = VEHICLE::GET_VEHICLE_LIVERY(Handle);
            break;
        case ModType::ModColor1:
            VEHICLE::GET_VEHICLE_MOD_COLOR_1(Handle, &Value0, &Value1, &Value2);
            break;
        case ModType::ModColor2:
            VEHICLE::GET_VEHICLE_MOD_COLOR_2(Handle, &Value0, &Value1);
            break;
        case ModType::Extra:
            if (VEHICLE::DOES_EXTRA_EXIST(Handle, ModId))
                Value0 = VEHICLE::IS_VEHICLE_EXTRA_TURNED_ON(Handle, ModId);
            break;
        case ModType::Neon:
            Value0 = VEHICLE::GET_VEHICLE_NEON_ENABLED(Handle, ModId);
            break;
        case ModType::NeonColor:
            VEHICLE::GET_VEHICLE_NEON_COLOUR(Handle, &Value0, &Value1, &Value2);
            break;
        case ModType::XenonColor:
            Value0 = VEHICLE::_GET_VEHICLE_XENON_LIGHTS_COLOUR(Handle);
            break;
        case ModType::PlateType:
            Value0 = VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(Handle);
            break;
        case ModType::InteriorColor:
            VEHICLE::_GET_VEHICLE_INTERIOR_COLOUR(Handle, &Value0);
            break;
        case ModType::DashboardColor:
            VEHICLE::_GET_VEHICLE_DASHBOARD_COLOUR(Handle, &Value0);
            break;
        default:;
    }
    return *this;
}

void VehicleModData::VehicleMod::Set() {
    switch (Type) {
        case ModType::Color:
            VEHICLE::SET_VEHICLE_COLOURS(Handle, Value0, Value1);
            break;
        case ModType::ExtraColors:
            VEHICLE::SET_VEHICLE_EXTRA_COLOURS(Handle, Value0, Value1);
            break;
        case ModType::ColorCustom1:
            VEHICLE::CLEAR_VEHICLE_CUSTOM_PRIMARY_COLOUR(Handle);
            if (Value0)
                VEHICLE::SET_VEHICLE_CUSTOM_PRIMARY_COLOUR(Handle, Value1, Value2, Value3);
            break;
        case ModType::ColorCustom2:
            VEHICLE::CLEAR_VEHICLE_CUSTOM_SECONDARY_COLOUR(Handle);
            if (Value0)
                VEHICLE::SET_VEHICLE_CUSTOM_SECONDARY_COLOUR(Handle, Value1, Value2, Value3);
            break;
        case ModType::Mod:
            VEHICLE::SET_VEHICLE_MOD(Handle, ModId, Value0, Value1);
            break;
        case ModType::ModToggle:
            VEHICLE::TOGGLE_VEHICLE_MOD(Handle, ModId, Value0);
            break;
        case ModType::WheelType:
            VEHICLE::SET_VEHICLE_WHEEL_TYPE(Handle, Value0);
            break;
        case ModType::WindowTint:
            VEHICLE::SET_VEHICLE_WINDOW_TINT(Handle, Value0);
            break;
        case ModType::TyresCanBurst:
            VEHICLE::SET_VEHICLE_TYRES_CAN_BURST(Handle, Value0);
            break;
        case ModType::TyreSmoke:
            VEHICLE::SET_VEHICLE_TYRE_SMOKE_COLOR(Handle, Value0, Value1, Value2);
            break;
        case ModType::Livery:
            VEHICLE::SET_VEHICLE_LIVERY(Handle, Value0);
            break;
        case ModType::ModColor1:
            VEHICLE::SET_VEHICLE_MOD_COLOR_1(Handle, Value0, Value1, Value2);
            break;
        case ModType::ModColor2:
            VEHICLE::SET_VEHICLE_MOD_COLOR_2(Handle, Value0, Value1);
            break;
        case ModType::Extra:
            if (VEHICLE::DOES_EXTRA_EXIST(Handle, ModId))
                VEHICLE::SET_VEHICLE_EXTRA(Handle, ModId, !Value0);
            break;
        case ModType::Neon:
            VEHICLE::SET_VEHICLE_NEON_ENABLED(Handle, ModId, Value0);
            break;
        case ModType::NeonColor:
            VEHICLE::SET_VEHICLE_NEON_COLOUR(Handle, Value0, Value1, Value2);
            break;
        case ModType::XenonColor:
            VEHICLE::_SET_VEHICLE_XENON_LIGHTS_COLOUR(Handle, Value0);
            break;
        case ModType::PlateType:
            VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(Handle, Value0);
            break;
        case ModType::InteriorColor:
            VEHICLE::_SET_VEHICLE_INTERIOR_COLOUR(Handle, Value0);
            break;
        case ModType::DashboardColor:
            VEHICLE::_SET_VEHICLE_DASHBOARD_COLOUR(Handle, Value0);
            break;
        default:;
    }
}

void VehicleModData::GetAll() {
    if (!Handle)
        return;

    VehicleMods.clear();

    Plate = VEHICLE::GET_VEHICLE_NUMBER_PLATE_TEXT(Handle);

    // Wheeltype
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::WheelType }.Get());

    // 0 t/m 24: normal; 25 t/m 49: Benny's
    for (uint8_t i = 0; i < 50; ++i) {
        if (i >= 17 && i <= 22) {
            VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::ModToggle, i }.Get());
        }
        else {
            VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::Mod, i }.Get());
        }
    }

    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::TyresCanBurst }.Get());
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::TyreSmoke }.Get());
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::PlateType }.Get());

    for (uint8_t i = 0; i <= 60; ++i) {
        VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::Extra, i }.Get());
    }

    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::Livery }.Get());

    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::WindowTint }.Get());

    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::Neon, 0 }.Get());
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::Neon, 1 }.Get());
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::Neon, 2 }.Get());
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::Neon, 3 }.Get());
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::NeonColor }.Get());

    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::ModColor1 }.Get());
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::ModColor2 }.Get());

    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::XenonColor }.Get());
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::InteriorColor }.Get());
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::DashboardColor }.Get());

    // Custom colors
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::ColorCustom1 }.Get());
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::ColorCustom2 }.Get());

    // Primary/Secondary
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::Color }.Get());
    VehicleMods.push_back(VehicleMod{ Handle, VehicleMod::ModType::ExtraColors }.Get());

}

void VehicleModData::SetAll() {
    if (!Handle)
        return;

    VEHICLE::SET_VEHICLE_NUMBER_PLATE_TEXT(Handle, Plate.c_str());

    VEHICLE::SET_VEHICLE_MOD_KIT(Handle, 0);

    for (auto& vehicleMod : VehicleMods) {
        vehicleMod.Handle = Handle;
        vehicleMod.Set();
    }
}

namespace {
    // Mod slot IDs for the performance-related mods, and their maximum index.
    // Index -1 means "max out the number of available mods for this slot".
    constexpr int kModEngine = 11;
    constexpr int kModBrakes = 12;
    constexpr int kModTransmission = 13;
    constexpr int kModSuspension = 15;
    constexpr int kModArmor = 16;
    constexpr int kModTurbo = 18;
}

void VehicleModData::SetMaxPerformanceMods(Vehicle vehicle) {
    if (!vehicle)
        return;

    VEHICLE::SET_VEHICLE_MOD_KIT(vehicle, 0);

    VEHICLE::SET_VEHICLE_MOD(vehicle, kModEngine, 3, false);
    VEHICLE::SET_VEHICLE_MOD(vehicle, kModBrakes, 2, false);
    VEHICLE::SET_VEHICLE_MOD(vehicle, kModTransmission, 2, false);
    VEHICLE::SET_VEHICLE_MOD(vehicle, kModSuspension, 3, false);
    VEHICLE::SET_VEHICLE_MOD(vehicle, kModArmor, 4, false);

    // Turbo is a toggle mod on most vehicles, some report a regular mod slot
    // with a single (index 0) option instead. Cover both cases.
    VEHICLE::TOGGLE_VEHICLE_MOD(vehicle, kModTurbo, true);
    if (VEHICLE::GET_NUM_VEHICLE_MODS(vehicle, kModTurbo) > 0) {
        VEHICLE::SET_VEHICLE_MOD(vehicle, kModTurbo, 0, false);
    }
}
