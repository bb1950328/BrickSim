#pragma once

#include "../errors/exceptions.h"
#include "../helpers/stringutil.h"
#include "../ldr/config.h"
#include "pugixml.hpp"
#include <chrono>

namespace bricksim::ldraw_library_updater {
    class UpdateFailedException : public errors::TaskFailedException {
    public:
        UpdateFailedException(const std::string& message, const std::source_location location = std::source_location::current());
    };

    enum class Step {
        INACTIVE,
        INITIALIZING,
        CHOOSE_ACTION,
        UPDATE_INCREMENTAL,
        UPDATE_COMPLETE,
        FINISHED,
    };
    struct Distribution {
        std::string id;
        std::chrono::year_month_day date;
        std::string url;
        std::size_t size;
        std::array<std::byte, 16> md5;
    };

    class UpdateState {
    public:
        Step step;

        float initializingProgress = 0.f;

        std::string currentReleaseId;
        std::chrono::year_month_day currentReleaseDate;

        std::vector<Distribution> incrementalUpdates;
        std::vector<float> incrementalUpdateProgress;

        std::optional<Distribution> completeDistribution;
        std::optional<float> completeUpdateProgress;

        std::size_t getIncrementalUpdateTotalSize() const;

        void initialize();

        void doIncrementalUpdate();

        void doCompleteUpdate();

    private:
        void findCurrentRelease();
        void readUpdatesList();
    };

    UpdateState& getState();
    void resetState();
}