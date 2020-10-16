//
// Created by Bader on 15.10.2020.
//

#include "gears.h"


gears::Gear::Gear(const int id,
                  const int numTeeth,
                  const int radiusLdu,
                  const gears::GearType type,
                  std::set<std::string> ldrParts) : id(id),
                                                    numTeeth(numTeeth),
                                                    radiusLDU(radiusLdu),
                                                    type(type),
                                                    ldrParts(std::move(ldrParts)) {

}

util::Fraction gears::GearPair::getRatio() {
    return util::Fraction(driver.numTeeth, follower.numTeeth);
}

gears::GearPair::GearPair(const gears::Gear &driver, const gears::Gear &follower) : driver(driver), follower(follower) {

}

bool gears::GearPair::is_valid() const {
    //todo maybe a 2d bool table is faster
    if (follower.type==GearType::WORM) {
        return false;
    }
    if (follower.type==GearType::INTERNAL_SPUR) {
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
    } else if (follower.type==GearType::EXTERNAL_BEVEL) {
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
    } else if (follower.type==GearType::EXTERNAL_SPUR) {
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

bool gears::GearPair::is_possible_on_liftbeam() const {
    return (driver.radiusLDU+follower.radiusLDU)%20==0;
}
