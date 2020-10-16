//
// Created by Bader on 15.10.2020.
//

#ifndef BRICKSIM_GEARS_H
#define BRICKSIM_GEARS_H

#include <string>
#include <set>
#include "../util.h"

namespace gears {
    enum class GearType {
        EXTERNAL_SPUR,
        INTERNAL_SPUR,
        EXTERNAL_SPUR_AND_SINGLE_BEVEL,
        EXTERNAL_DOUBLE_BEVEL,
        EXTERNAL_BEVEL,
        WORM
    };

    class Gear {
    private:
        const int id;
    public:
        Gear(int id, int numTeeth, int radiusLdu, GearType type, std::set<std::string> ldrParts);
        Gear()=delete;
        Gear & operator=(const Gear&) = delete;
        Gear(const Gear&) = delete;

        const int numTeeth;
        const int radiusLDU;
        const GearType type;
        const std::set<std::string> ldrParts;
    };

    class GearPair {
    private:
        const Gear& driver;
        const Gear& follower;
    public:
        GearPair(const Gear &driver, const Gear &follower);

        util::Fraction getRatio();

        [[nodiscard]] bool is_valid() const;
        [[nodiscard]] bool is_possible_on_liftbeam() const;
    };

    [[maybe_unused]] const Gear WORM_GEAR                   (0, 1, 10, GearType::WORM, {"15457.dat", "4716.dat", "32905.dat"});
    [[maybe_unused]] const Gear WORM_GEAR_SHORT             (1, 1, 15, GearType::WORM, {"27938 .dat"});
    [[maybe_unused]] const Gear GEAR_8T                     (2, 8, 10, GearType::EXTERNAL_SPUR, {"3647.dat", "10928.dat", "11955.dat"});
    [[maybe_unused]] const Gear GEAR_12T_BEVEL              (3, 12, 15, GearType::EXTERNAL_BEVEL, {"6589.dat"});
    [[maybe_unused]] const Gear GEAR_12T_DOUBLE_BEVEL       (4, 12, 15, GearType::EXTERNAL_DOUBLE_BEVEL, {"32270.dat"});
    [[maybe_unused]] const Gear GEAR_16T                    (5, 16, 20, GearType::EXTERNAL_SPUR, {"4019.dat", "94925.dat", "6542.dat", "18946.dat", "6542b.dat"});
    [[maybe_unused]] const Gear GEAR_20T_BEVEL              (6, 20, 25, GearType::EXTERNAL_BEVEL, {"32198.dat", "87407.dat"});
    [[maybe_unused]] const Gear GEAR_20T_DOUBLE_BEVEL       (7, 20, 25, GearType::EXTERNAL_DOUBLE_BEVEL, {"32269.dat", "35185.dat"});
    [[maybe_unused]] const Gear GEAR_24T                    (8, 24, 30, GearType::EXTERNAL_SPUR, {"x187.dat", "3648.dat", "60c01.dat", "3650a.dat", "3650b.dat", "3650.dat"});//todo part 3650 is also a crown gear
    [[maybe_unused]] const Gear GEAR_TURNTABLE_INSIDE_24T   (9, 24, 30, GearType::INTERNAL_SPUR, {"2856.dat", "48452.dat"});
    [[maybe_unused]] const Gear GEAR_28T_DOUBLE_BEVEL       (10, 28, 35, GearType::EXTERNAL_DOUBLE_BEVEL, {"46372.dat", "65413.dat"});
    [[maybe_unused]] const Gear GEAR_TURNTABLE_28T          (11, 28, 35, GearType::EXTERNAL_SPUR_AND_SINGLE_BEVEL, {"99010.dat"});
    [[maybe_unused]] const Gear GEAR_DIFFERENTIAL_28T       (12, 28, 35, GearType::EXTERNAL_BEVEL, {"62821a.dat", "62821b.dat", "62821u.dat"});
    [[maybe_unused]] const Gear GEAR_36T_DOUBLE_BEVEL       (13, 36, 45, GearType::EXTERNAL_DOUBLE_BEVEL, {"32498.dat"});
    [[maybe_unused]] const Gear GEAR_40T                    (14, 40, 50, GearType::EXTERNAL_SPUR, {"3649.dat"});
    [[maybe_unused]] const Gear GEAR_POWER_MINERS_WHEEL_48T (15, 48, 60, GearType::INTERNAL_SPUR, {"64712.dat"});
    [[maybe_unused]] const Gear GEAR_TURNTABLE_56T          (16, 56, 85, GearType::EXTERNAL_SPUR, {"2855.dat", "48168.dat"});
    [[maybe_unused]] const Gear GEAR_TURNTABLE_60T          (17, 60, 90, GearType::EXTERNAL_SPUR_AND_SINGLE_BEVEL, {"18938.dat"});
    [[maybe_unused]] const Gear GEAR_FOUR_CURVED_RACKS_140T (18, 140, 175, GearType::INTERNAL_SPUR, {"24121.dat"});
    [[maybe_unused]] const Gear GEAR_HAILFIRE_DROID_168T    (19, 168, 210, GearType::INTERNAL_SPUR, {"x784.dat"});
}

#endif //BRICKSIM_GEARS_H
