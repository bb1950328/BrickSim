#include "snap_rotational.h"
#include "../config.h"
#include <cmath>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <utility>

namespace bricksim::snap {
    RotationalSnapStepPreset::RotationalSnapStepPreset(std::string name, float stepDeg) :
        name(std::move(name)), stepDeg(stepDeg) {}
    const std::vector<RotationalSnapStepPreset>& RotationalHandler::getPresets() const {
        return presets;
    }
    const RotationalSnapStepPreset& RotationalHandler::getTemporaryPreset() const {
        return temporaryPreset;
    }
    int RotationalHandler::getCurrentPresetIndex() const {
        return currentPresetIndex;
    }
    void RotationalHandler::setCurrentPresetIndex(int value) {
        currentPresetIndex = value;
    }
    void RotationalHandler::setTemporaryPreset(const RotationalSnapStepPreset& value) {
        temporaryPreset = value;
    }
    void RotationalHandler::init() {
        rapidjson::Document d;
        d.Parse(config::get(config::ROTATIONAL_SNAP_PRESETS).c_str());
        for (const auto& item: d.GetArray()) {
            presets.emplace_back(item["name"].GetString(), item["step"].GetFloat());
        }
        const float configStep = config::get(config::ROTATIONAL_SNAP_STEP);
        for (int i = 0; i < presets.size(); ++i) {
            if (std::fabs(presets[i].stepDeg - configStep) < .01f) {
                currentPresetIndex = i;
                break;
            }
        }
        if (currentPresetIndex == TEMPORARY_PRESET_INDEX) {
            setTemporaryPreset({"", configStep});
        }
    }
    void RotationalHandler::cleanup() {
        config::set(config::ROTATIONAL_SNAP_STEP, getCurrentPreset().stepDeg);

        rapidjson::Document d;
        rapidjson::Document::AllocatorType& allocator = d.GetAllocator();
        d.SetArray();
        for (const auto& item: presets) {
            rapidjson::Value obj(rapidjson::kObjectType);

            rapidjson::Value nameVal;
            nameVal.SetString(item.name.c_str(), item.name.length(), allocator);
            obj.AddMember("name", nameVal, allocator);

            rapidjson::Value xzVal;
            xzVal.SetFloat(item.stepDeg);
            obj.AddMember("step", xzVal, allocator);

            d.PushBack(obj, allocator);
        }
        rapidjson::StringBuffer strbuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
        d.Accept(writer);
        config::set(config::ROTATIONAL_SNAP_PRESETS, strbuf.GetString());
    }
    float RotationalHandler::getNearestValue(float initialValueDeg, float currentValueDeg) const {
        const auto step = getCurrentPreset().stepDeg;
        const auto delta = currentValueDeg - initialValueDeg;
        const auto factor = std::round(delta / step);
        return std::fmod(initialValueDeg + factor * step, 360.f);
    }
    const RotationalSnapStepPreset& RotationalHandler::getCurrentPreset() const {
        return presets[currentPresetIndex];
    }
}
