#include "snap_linear.h"
#include "../config.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>

namespace bricksim::snap {
    void LinearHandler::init() {
        rapidjson::Document d;
        d.Parse(config::get(config::LINEAR_SNAP_PRESETS).c_str());
        for (const auto& item: d.GetArray()) {
            presets.emplace_back(item["name"].GetString(), item["xz"].GetInt(), item["y"].GetInt());
        }
        const int configXZ = config::get(config::LINEAR_SNAP_STEP_XZ);
        const int configY = config::get(config::LINEAR_SNAP_STEP_Y);
        for (size_t i = 0; i < presets.size(); ++i) {
            if (presets[i].stepXZ == configXZ && presets[i].stepY == configY) {
                currentPresetIndex = i;
                break;
            }
        }
        if (currentPresetIndex == TEMPORARY_PRESET_INDEX) {
            setTemporaryPreset({"", configXZ, configY});
        }
    }
    const std::vector<LinearSnapStepPreset>& LinearHandler::getPresets() const {
        return presets;
    }
    int LinearHandler::getCurrentPresetIndex() const {
        return currentPresetIndex;
    }
    void LinearHandler::setCurrentPresetIndex(int newPreset) {
        currentPresetIndex = newPreset;
    }
    const LinearSnapStepPreset& LinearHandler::getCurrentPreset() const {
        return currentPresetIndex < 0
                       ? temporaryPreset
                       : presets[currentPresetIndex];
    }
    void LinearHandler::setTemporaryPreset(const LinearSnapStepPreset& newTmpPreset) {
        temporaryPreset = newTmpPreset;
    }
    void LinearHandler::cleanup() {
        config::set(config::LINEAR_SNAP_STEP_XZ, getCurrentPreset().stepXZ);
        config::set(config::LINEAR_SNAP_STEP_Y, getCurrentPreset().stepY);

        rapidjson::Document d;
        rapidjson::Document::AllocatorType& allocator = d.GetAllocator();
        d.SetArray();
        for (const auto& item: presets) {
            rapidjson::Value obj(rapidjson::kObjectType);

            rapidjson::Value nameVal;
            nameVal.SetString(item.name.c_str(), item.name.length(), allocator);
            obj.AddMember("name", nameVal, allocator);

            rapidjson::Value xzVal;
            xzVal.SetInt(item.stepXZ);
            obj.AddMember("xz", xzVal, allocator);

            rapidjson::Value yVal;
            yVal.SetInt(item.stepY);
            obj.AddMember("y", yVal, allocator);

            d.PushBack(obj, allocator);
        }
        rapidjson::StringBuffer strbuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
        d.Accept(writer);
        config::set(config::LINEAR_SNAP_PRESETS, strbuf.GetString());
    }
    const LinearSnapStepPreset& LinearHandler::getTemporaryPreset() const {
        return temporaryPreset;
    }
    LinearSnapStepPreset::LinearSnapStepPreset(std::string name, const int distanceXz, const int distanceY) :
        SnapStepPreset(name), stepXZ(distanceXz), stepY(distanceY) {}
    glm::ivec3 LinearSnapStepPreset::stepXYZ() const {
        return {stepXZ, stepY, stepXZ};
    }
    std::optional<gui::icons::IconType> LinearSnapStepPreset::getIcon() const {
        using gui::icons::IconType;
        if (stepXZ == 20) {
            if (stepY == 24) {
                return IconType::Brick1Side;
            } else if (stepY == 20) {
                return IconType::Brick1x1Top;
            }
        } else if (stepXZ == 10) {
            if (stepY == 8) {
                return IconType::Plate1Side;
            } else if (stepY == 10) {
                return IconType::Plate1HalfTop;
            }
        }
        return std::nullopt;
    }
}
