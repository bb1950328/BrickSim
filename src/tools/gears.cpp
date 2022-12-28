#include "gears.h"
#include <map>

namespace bricksim::gears {
    Gear::Gear(const int id,
               const char* description,
               const int numTeeth,
               const int radiusLdu,
               const GearType type,
               uoset_t<const char*> ldrParts) :
        id(id),
        description(description),
        numTeeth(numTeeth),
        radiusLDU(radiusLdu),
        type(type),
        ldrParts(std::move(ldrParts)) {
    }

    bool Gear::operator==(const Gear& rhs) const {
        return id == rhs.id;
    }

    Fraction GearPair::getRatio() const {
        return Fraction(driver->numTeeth, follower->numTeeth);
    }

    GearPair::GearPair(const gear_t& driver, const gear_t& follower) :
        driver(driver), follower(follower) {
    }

    bool GearPair::isValid() const {
        using enum GearType;
        //todo maybe a 2d bool table is faster
        if (follower->type == GearType::WORM) {
            return false;
        }
        if (follower->type == GearType::INTERNAL_SPUR) {
            switch (driver->type) {
                case WORM:
                case INTERNAL_SPUR:
                case EXTERNAL_BEVEL: return false;
                case EXTERNAL_DOUBLE_BEVEL:
                case EXTERNAL_SPUR_AND_SINGLE_BEVEL:
                case EXTERNAL_SPUR: return true;
            }
        } else if (follower->type == EXTERNAL_BEVEL) {
            switch (driver->type) {
                case WORM:
                case EXTERNAL_SPUR:
                case INTERNAL_SPUR: return false;
                case EXTERNAL_SPUR_AND_SINGLE_BEVEL:
                case EXTERNAL_DOUBLE_BEVEL:
                case EXTERNAL_BEVEL: return true;
            }
        } else if (follower->type == EXTERNAL_SPUR) {
            switch (driver->type) {
                case EXTERNAL_BEVEL: return false;
                case EXTERNAL_SPUR_AND_SINGLE_BEVEL:
                case EXTERNAL_DOUBLE_BEVEL:
                case WORM:
                case EXTERNAL_SPUR:
                case INTERNAL_SPUR: return true;
            }
        }
        return true;
    }

    bool GearPair::isPossibleOnLiftbeam() const {
        return (driver->radiusLDU + follower->radiusLDU) % 20 == 0;
    }

    const gear_t& GearPair::getDriver() const {
        return driver;
    }

    const gear_t& GearPair::getFollower() const {
        return follower;
    }

    gear_collection_t& getAllGearsOfType(GearType type) {
        static omap_t<GearType, oset_t<gear_t>> byType;
        if (byType.empty()) {
            for (const auto& gear: ALL_GEARS) {
                byType[gear->type].insert(gear);
            }
        }
        return byType[type];
    }
}
