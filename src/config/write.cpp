#include "write.h"
#include <fstream>

namespace bricksim::config {
    namespace {
        const std::string JSON_FILE_NAME = "config.json";
        std::unique_ptr<Config> data;
    }

    void initialize() {
        data = std::make_unique<Config>();
        if (std::filesystem::exists(JSON_FILE_NAME)) {
            std::ifstream ifs(JSON_FILE_NAME);
            rapidjson::IStreamWrapper isw(ifs);
            rapidjson::Document d;
            d.ParseStream(isw);
            json_dto::json_input_t jin(d);
            data->json_io(jin);
        }
    }

    void save() {
        std::ofstream ofs(JSON_FILE_NAME);
        rapidjson::OStreamWrapper osw(ofs);
        rapidjson::Document d;
        json_dto::json_output_t jout(d, d.GetAllocator());
        data->json_io(jout);
        rapidjson::PrettyWriter writer(osw);
        d.Accept(writer);
    }

    Config& getMutable() {
        return *data;
    }
}
