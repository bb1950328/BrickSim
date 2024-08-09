#include "ldraw_library_updater.h"
#include "../helpers/util.h"
#include "../ldr/config.h"
#include "../ldr/file_repo.h"
#include "pugixml.hpp"
#include "spdlog/fmt/fmt.h"

namespace bricksim::ldraw_library_updater {
    namespace {
        std::unique_ptr<UpdateState> state;

        static constexpr std::size_t ESTIMATED_UPDATE_FILE_SIZE = 45000;

        int updatesListDownloadProgress([[maybe_unused]] void* ptr, [[maybe_unused]] long downTotal, long downNow, [[maybe_unused]] long upTotal, [[maybe_unused]] long upNow) {
            if (state != nullptr) {
                state->initializingProgress = .1f + std::max(.9f, .9f*downNow/ESTIMATED_UPDATE_FILE_SIZE);
            }
            return 0;
        }
        Distribution parseDistribution(const pugi::xml_node& distNode) {
            const auto* releaseDate = distNode.child("release_date").child_value();
            const auto releaseDateParsed = std::strlen(releaseDate)==10
                                                   ? stringutil::parseYYYY_MM_DD(std::string_view(releaseDate))
                                                   : std::chrono::year_month_day();
            const auto* releaseId = distNode.child("release_id").child_value();
            const auto* url = distNode.child("url").child_value();
            std::size_t size;
            sscanf(distNode.child("size").child_value(), "%zu", &size);
            const auto* md5hex = distNode.child("md5_fingerprint").child_value();
            std::array<std::byte, 16> md5{static_cast<std::byte>(0)};
            for (int i = 0; i < 16; ++i) {
                for (int j = 0; j < 2; ++j) {
                    const auto c = md5hex[i*2+j];
                    const auto nibble = c <= '9'?c-'0':c-'a'+10;
                    md5[i] = md5[i]<<4 + nibble;
                }
            }
            return {releaseId, releaseDateParsed, url, size, md5};
        }
    }

    UpdateState& getState() {
        if (state== nullptr) {
            state = std::make_unique<UpdateState>();
        }
        return *state;
    }

    UpdateFailedException::UpdateFailedException(const std::string& message, const std::source_location location) :
        TaskFailedException(message, location) {}

    void UpdateState::initialize() {
        if (step != Step::INACTIVE) {
            throw std::invalid_argument("wrong time to call this method");
        }
        step = Step::INITIALIZING;
        initializingProgress = .05f;
        findCurrentRelease();
        initializingProgress = .1f;
        readUpdatesList();
        initializingProgress = 1.f;
        step = Step::CHOOSE_ACTION;
    }
    void UpdateState::findCurrentRelease() {
        currentReleaseDate = stringutil::parseYYYY_MM_DD(ldr::getConfig().getUpdateDate());
    }
    void UpdateState::readUpdatesList() {
        const auto [statusCode, xmlContent] = util::requestGET(constants::LDRAW_LIBRARY_UPDATES_XML_URL, false, 0, updatesListDownloadProgress);
        if (statusCode < 200 | statusCode >= 300) {
            throw UpdateFailedException(fmt::format("download of update info XML failed with status {}", statusCode));
        }
        pugi::xml_document doc;
        const auto parseResult = doc.load_buffer(xmlContent.c_str(), xmlContent.size());
        if (parseResult.status!=pugi::status_ok) {
            throw UpdateFailedException(fmt::format("error while parsing update info XML at char {}. {}", parseResult.offset, parseResult.description()));
        }
        for (auto distNode = doc.root().first_child().child("distribution"); distNode; distNode = distNode.next_sibling("distribution")) {
            const auto* fileFormat = distNode.child("file_format").child_value();
            if (std::strcmp(fileFormat, "ZIP")==0) {
                const auto* releaseType = distNode.child("release_type").child_value();
                const auto distribution = parseDistribution(distNode);

                if (std::strcmp(releaseType, "UPDATE")==0) {
                    if (distribution.date>currentReleaseDate) {
                        incrementalUpdates.push_back(distribution);
                    }
                } else if (std::strcmp(releaseType, "COMPLETE")==0) {
                    completeDistribution = distribution;
                }
            }
        }
    }
    std::size_t UpdateState::getIncrementalUpdateTotalSize() const {
        return std::accumulate(incrementalUpdates.cbegin(),
                               incrementalUpdates.cend(),
                               std::size_t(0),
                               [](std::size_t x, const Distribution& dist){return x+dist.size;});
    }
    void UpdateState::doIncrementalUpdate() {
        //todo implement
    }
    void UpdateState::doCompleteUpdate() {
        //todo implement
    }
}