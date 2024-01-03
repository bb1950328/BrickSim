#pragma once
#include "magic_enum.hpp"
#include "../types.h"
#include "json_dto/pub.hpp"
#include "spdlog/fmt/fmt.h"

namespace bricksim::json_helper {
    class JsonValidationError final : public std::exception {
        const std::string message;

    public:
        explicit JsonValidationError(const std::string& message) :
            message(message) {}

        const char* what() const noexcept override;
    };

    template<typename T>
    void init(T* this_, const char* const json) {
        rapidjson::Document document;
        document.Parse(json);
        json_dto::json_input_t jin{document};
        this_->json_io(jin);
    }

    template<typename T>
    void defaultInit(T* this_) {
        init(this_, "{}");//todo find a way that does not call rapidjson::Document#Parse
    }

    template<typename E>
    struct EnumRW {
        void read(E& value, const rapidjson::Value& from) const {
            using json_dto::read_json_value;
            std::string representation;
            read_json_value(representation, from);
            const auto opt = magic_enum::enum_cast<E>(representation);
            if (!opt.has_value()) {
                std::string examples = "";
                uint64_t listed = 0;
                for (const auto& name: magic_enum::enum_names<E>()) {
                    if (!examples.empty()) {
                        examples += ", ";
                    }
                    examples += name;
                    ++listed;
                    if (listed > 10) {
                        break;
                    }
                }
                if (magic_enum::enum_count<E>() > listed) {
                    examples += " etc.";
                }
                throw JsonValidationError(fmt::format("invalid enum value \"{}\". Possible values are {}", representation, examples));
            }
            value = *opt;
        }

        void write(const E& value, rapidjson::Value& object, rapidjson::MemoryPoolAllocator<>& allocator) const {
            using json_dto::write_json_value;
            write_json_value(std::string(magic_enum::enum_name(value)), object, allocator);
        }
    };

    template<typename E>
    struct EnumSetRW {
        EnumRW<E> single;
        void read(uoset_t<E>& value, const rapidjson::Value& from) const {
            for (const auto & item : from.GetArray()) {
                E result;
                single.read(result, item);
                value.insert(result);
            }
        }

        void write(const uoset_t<E>& value, rapidjson::Value& object, rapidjson::MemoryPoolAllocator<>& allocator) const {
            object.SetArray();
            for (const auto & item : value) {
                rapidjson::Value result;
                single.write(item, result, allocator);
                object.GetArray().PushBack(result, allocator);
            }
        }
    };
}
