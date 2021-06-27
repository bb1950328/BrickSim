#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-err58-cpp"
#ifndef BRICKSIM_GEARS_H
#define BRICKSIM_GEARS_H

#include <memory>
#include <set>
#include "../helpers/fraction.h"

namespace bricksim::gears {
    enum class GearType {
        EXTERNAL_SPUR,
        INTERNAL_SPUR,
        EXTERNAL_SPUR_AND_SINGLE_BEVEL,
        EXTERNAL_DOUBLE_BEVEL,
        EXTERNAL_BEVEL,
        WORM
    };

    class Gear : public std::enable_shared_from_this<Gear> {
    public:
        const int id;
        Gear(int id, const char* description, int numTeeth, int radiusLdu, GearType type, std::set<const char*> ldrParts);
        Gear()=delete;
        Gear & operator=(const Gear&) = delete;
        Gear(const Gear&) = delete;
        bool operator==(const Gear &rhs) const;
        bool operator!=(const Gear &rhs) const;

        const char* const description;
        const int numTeeth;
        const int radiusLDU;
        const GearType type;
        const std::set<const char*> ldrParts;
    };

    typedef std::shared_ptr<Gear> gear_t;
    typedef const std::set<gear_t> gear_collection_t;

    class GearPair {
    private:
        const gear_t driver;
        const gear_t follower;
    public:
        GearPair(const gear_t &driver, const gear_t &follower);

        [[nodiscard]] Fraction getRatio() const;

        [[nodiscard]] bool isValid() const;
        [[nodiscard]] bool isPossibleOnLiftbeam() const;

        [[nodiscard]] const gear_t &getDriver() const;

        [[nodiscard]] const gear_t &getFollower() const;
    };

    [[nodiscard]] gear_collection_t& getAllGearsOfType(GearType type);

    const gear_t WORM_GEAR                   = std::make_shared<Gear>(0, "Worm thin",                       1, 10, GearType::WORM, std::set({"15457.dat", "4716.dat", "32905.dat"}));
    const gear_t WORM_GEAR_SHORT             = std::make_shared<Gear>(1, "Worm thick",                      1, 15, GearType::WORM, std::set({"27938 .dat"}));
    const gear_t GEAR_8T                     = std::make_shared<Gear>(2, "Spur 8T",                         8, 10, GearType::EXTERNAL_SPUR, std::set({"3647.dat", "10928.dat", "11955.dat"}));
    const gear_t GEAR_12T_BEVEL              = std::make_shared<Gear>(3, "Bevel 12T",                       12, 15, GearType::EXTERNAL_BEVEL, std::set({"6589.dat"}));
    const gear_t GEAR_12T_DOUBLE_BEVEL       = std::make_shared<Gear>(4, "Double Bevel 12T",                12, 15, GearType::EXTERNAL_DOUBLE_BEVEL, std::set({"32270.dat"}));
    const gear_t GEAR_16T                    = std::make_shared<Gear>(5, "Spur 16T",                        16, 20, GearType::EXTERNAL_SPUR, std::set({"4019.dat", "94925.dat", "6542.dat", "18946.dat", "6542b.dat"}));
    const gear_t GEAR_20T_BEVEL              = std::make_shared<Gear>(6, "Bevel 20T",                       20, 25, GearType::EXTERNAL_BEVEL, std::set({"32198.dat", "87407.dat"}));
    const gear_t GEAR_20T_DOUBLE_BEVEL       = std::make_shared<Gear>(7, "Double Bevel 20T",                20, 25, GearType::EXTERNAL_DOUBLE_BEVEL, std::set({"32269.dat", "35185.dat"}));
    const gear_t GEAR_24T                    = std::make_shared<Gear>(8, "Spur 24T",                        24, 30, GearType::EXTERNAL_SPUR, std::set({"x187.dat", "3648.dat", "60c01.dat", "3650a.dat", "3650b.dat", "3650.dat"}));//todo part 3650 is also a crown gear
    const gear_t GEAR_TURNTABLE_INSIDE_24T   = std::make_shared<Gear>(9, "Turntable inside 24T",            24, 30, GearType::INTERNAL_SPUR, std::set({"2856.dat", "48452.dat"}));
    const gear_t GEAR_28T_DOUBLE_BEVEL       = std::make_shared<Gear>(10, "Double Bevel 28T",               28, 35, GearType::EXTERNAL_DOUBLE_BEVEL, std::set({"46372.dat", "65413.dat"}));
    const gear_t GEAR_TURNTABLE_28T          = std::make_shared<Gear>(11, "Small Turntable 28T",            28, 35, GearType::EXTERNAL_SPUR_AND_SINGLE_BEVEL, std::set({"99010.dat"}));
    const gear_t GEAR_DIFFERENTIAL_28T       = std::make_shared<Gear>(12, "Differential 28T",               28, 35, GearType::EXTERNAL_BEVEL, std::set({"62821a.dat", "62821b.dat", "62821u.dat"}));
    const gear_t GEAR_36T_DOUBLE_BEVEL       = std::make_shared<Gear>(13, "Double Bevel 36T",               36, 45, GearType::EXTERNAL_DOUBLE_BEVEL, std::set({"32498.dat"}));
    const gear_t GEAR_40T                    = std::make_shared<Gear>(14, "Spur 40T",                       40, 50, GearType::EXTERNAL_SPUR, std::set({"3649.dat"}));
    const gear_t GEAR_POWER_MINERS_WHEEL_48T = std::make_shared<Gear>(15, "Power Miners Wheel inside 48T",  48, 60, GearType::INTERNAL_SPUR, std::set({"64712.dat"}));
    const gear_t GEAR_TURNTABLE_56T          = std::make_shared<Gear>(16, "Turntable 56T",                  56, 85, GearType::EXTERNAL_SPUR, std::set({"2855.dat", "48168.dat"}));
    const gear_t GEAR_TURNTABLE_60T          = std::make_shared<Gear>(17, "Turntable 60T",                  60, 90, GearType::EXTERNAL_SPUR_AND_SINGLE_BEVEL, std::set({"18938.dat"}));
    const gear_t GEAR_FOUR_CURVED_RACKS_140T = std::make_shared<Gear>(18, "4 Curved Racks inside 140T",     140, 175, GearType::INTERNAL_SPUR, std::set({"24121.dat"}));
    const gear_t GEAR_HAILFIRE_DROID_168T    = std::make_shared<Gear>(19, "Hailfire Droid inside 168T",     168, 210, GearType::INTERNAL_SPUR, std::set({"x784.dat"}));
    gear_collection_t ALL_GEARS = {
            WORM_GEAR,
            WORM_GEAR_SHORT,
            GEAR_8T,
            GEAR_12T_BEVEL,
            GEAR_12T_DOUBLE_BEVEL,
            GEAR_16T,
            GEAR_20T_BEVEL,
            GEAR_20T_DOUBLE_BEVEL,
            GEAR_24T,
            GEAR_TURNTABLE_INSIDE_24T,
            GEAR_28T_DOUBLE_BEVEL,
            GEAR_TURNTABLE_28T,
            GEAR_DIFFERENTIAL_28T,
            GEAR_36T_DOUBLE_BEVEL,
            GEAR_40T,
            GEAR_POWER_MINERS_WHEEL_48T,
            GEAR_TURNTABLE_56T,
            GEAR_TURNTABLE_60T,
            GEAR_FOUR_CURVED_RACKS_140T,
            GEAR_HAILFIRE_DROID_168T,
    };
}

#endif //BRICKSIM_GEARS_H

#pragma clang diagnostic pop