

#include "gears.h"

namespace gears {
    Gear::Gear(const int id,
               const int numTeeth,
               const int radiusLdu,
               const GearType type,
               std::set<std::string> ldrParts) : id(id),
                                                 numTeeth(numTeeth),
                                                 radiusLDU(radiusLdu),
                                                 type(type),
                                                 ldrParts(std::move(ldrParts)) {

    }

    Fraction GearPair::getRatio() {
        return Fraction(driver.numTeeth, follower.numTeeth);
    }

    GearPair::GearPair(const Gear &driver, const Gear &follower) : driver(driver), follower(follower) {

    }

    bool GearPair::is_valid() const {
        //todo maybe a 2d bool table is faster
        if (follower.type == GearType::WORM) {
            return false;
        }
        if (follower.type == GearType::INTERNAL_SPUR) {
            switch (driver.type) {
                case GearType::WORM:
                case GearType::INTERNAL_SPUR:
                case GearType::EXTERNAL_BEVEL:
                    return false;
                case GearType::EXTERNAL_DOUBLE_BEVEL:
                case GearType::EXTERNAL_SPUR_AND_SINGLE_BEVEL:
                case GearType::EXTERNAL_SPUR:
                    return true;
            }
        } else if (follower.type == GearType::EXTERNAL_BEVEL) {
            switch (driver.type) {
                case GearType::WORM:
                case GearType::EXTERNAL_SPUR:
                case GearType::INTERNAL_SPUR:
                    return false;
                case GearType::EXTERNAL_SPUR_AND_SINGLE_BEVEL:
                case GearType::EXTERNAL_DOUBLE_BEVEL:
                case GearType::EXTERNAL_BEVEL:
                    return true;
            }
        } else if (follower.type == GearType::EXTERNAL_SPUR) {
            switch (driver.type) {
                case GearType::EXTERNAL_BEVEL:
                    return false;
                case GearType::EXTERNAL_SPUR_AND_SINGLE_BEVEL:
                case GearType::EXTERNAL_DOUBLE_BEVEL:
                case GearType::WORM:
                case GearType::EXTERNAL_SPUR:
                case GearType::INTERNAL_SPUR:
                    return true;
            }
        }
        return true;
    }

    bool GearPair::is_possible_on_liftbeam() const {
        return (driver.radiusLDU + follower.radiusLDU) % 20 == 0;
    }


    std::vector<const Gear *> get_all_gears() {
        //todo google how to make this a constant
        static auto result = std::vector<const Gear *>();
        if (result.empty()) {
            result.push_back(&WORM_GEAR_SHORT);
            result.push_back(&GEAR_8T);
            result.push_back(&GEAR_12T_BEVEL);
            result.push_back(&GEAR_12T_DOUBLE_BEVEL);
            result.push_back(&GEAR_16T);
            result.push_back(&GEAR_20T_BEVEL);
            result.push_back(&GEAR_20T_DOUBLE_BEVEL);
            result.push_back(&GEAR_24T);
            result.push_back(&GEAR_TURNTABLE_INSIDE_24T);
            result.push_back(&GEAR_28T_DOUBLE_BEVEL);
            result.push_back(&GEAR_TURNTABLE_28T);
            result.push_back(&GEAR_DIFFERENTIAL_28T);
            result.push_back(&GEAR_36T_DOUBLE_BEVEL);
            result.push_back(&GEAR_40T);
            result.push_back(&GEAR_POWER_MINERS_WHEEL_48T);
            result.push_back(&GEAR_TURNTABLE_56T);
            result.push_back(&GEAR_TURNTABLE_60T);
            result.push_back(&GEAR_FOUR_CURVED_RACKS_140T);
            result.push_back(&GEAR_HAILFIRE_DROID_168T);
        }
        return result;
    }
}
