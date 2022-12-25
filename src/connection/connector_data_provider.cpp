#include "connector_data_provider.h"
#include "../ldr/file_repo.h"
#include "ldcad_snap_meta/clear_command.h"
#include "ldcad_snap_meta/clp_command.h"
#include "ldcad_snap_meta/cyl_command.h"
#include "ldcad_snap_meta/incl_command.h"
namespace bricksim::connection {
    namespace {
        uomap_t<std::string, std::vector<std::shared_ptr<Connector>>> cache;

        void multiplyConnectorByGrid(std::vector<std::shared_ptr<Connector>>& connectors, std::vector<std::shared_ptr<Connector>>& base, const ldcad_snap_meta::Grid& grid);

        template<typename T>
        [[nodiscard]] glm::mat4 combinePosOri(const std::shared_ptr<T>& command) {
            glm::mat4 transf(1.f);
            if (command->pos.has_value()) {
                transf = glm::translate(transf, *command->pos);
            }
            if (command->ori.has_value()) {
                transf = glm::mat4(*command->ori) * transf;
            }
            return transf;
        }

        template<typename T>
        [[nodiscard]] glm::mat4 combinePosOriScale(const std::shared_ptr<T>& command) {
            glm::mat4 transf = combinePosOri(command);
            if (command->scale.has_value()) {
                transf = glm::scale(transf, *command->scale);
            }
            return transf;
        }

        void
        createConnectors(std::vector<std::shared_ptr<Connector>> &connectors, const std::shared_ptr<ldr::File> &file,
                         glm::mat4 const &transformation, uoset_t<std::string> clearIDs) {
            bool clearAll = false;
            for (const auto &command: file->ldcadSnapMetas) {
                const auto clearCommand = std::dynamic_pointer_cast<ldcad_snap_meta::ClearCommand>(command);
                if (clearCommand != nullptr) {
                    if (clearCommand->id.has_value()) {
                        clearIDs.insert(*clearCommand->id);
                    } else {
                        clearAll = true;
                        break;
                    }
                }

                const auto inclCommand = std::dynamic_pointer_cast<ldcad_snap_meta::InclCommand>(command);
                if (inclCommand != nullptr) {
                    glm::mat4 transf = combinePosOriScale(inclCommand);

                    const auto includedFile = ldr::file_repo::get().getFile(inclCommand->ref);

                    if (inclCommand->grid.has_value()) {
                        std::vector<std::shared_ptr<Connector>> base;
                        createConnectors(base, includedFile, transf * transformation, clearIDs);
                        const auto& grid = *inclCommand->grid;
                        multiplyConnectorByGrid(connectors, base, grid);
                    } else {
                        createConnectors(connectors, includedFile, transf * transformation, clearIDs);
                    }
                }

                const auto cylCommand = std::dynamic_pointer_cast<ldcad_snap_meta::CylCommand>(command);
                if (cylCommand != nullptr && (!cylCommand->id.has_value() || !clearIDs.contains(*cylCommand->id))) {
                    //TODO check cylCommand->scale
                    auto result = std::make_shared<CylindricalConnector>(
                            cylCommand->group.value_or(""),
                            combinePosOri(cylCommand) * transformation,
                            cylCommand->gender == ldcad_snap_meta::Gender::M
                            ? Gender::M
                            : Gender::F,
                            std::vector<CylindricalShapePart>(),
                            false,
                            false,
                            cylCommand->slide);
                    result->parts.reserve(cylCommand->secs.size());
                    for (const auto& sec: cylCommand->secs) {
                        result->parts.push_back({
                                CylindricalShapeType::ROUND,
                                false,
                                sec.radius,//todo scale these values
                                sec.length,
                        });
                    }
                    for (int i = 0; i < cylCommand->secs.size(); ++i) {
                        const auto& sec = cylCommand->secs[i];
                        auto& part = result->parts[i];
                        switch (sec.variant) {
                            case ldcad_snap_meta::CylShapeVariant::R:
                                part.type = CylindricalShapeType::ROUND;
                                break;
                            case ldcad_snap_meta::CylShapeVariant::A:
                                part.type = CylindricalShapeType::AXLE;
                                break;
                            case ldcad_snap_meta::CylShapeVariant::S:
                                part.type = CylindricalShapeType::SQUARE;
                                break;
                            default:
                                break;
                        }
                    }
                    for (int i = 0; i < cylCommand->secs.size(); ++i) {
                        const auto &sec = cylCommand->secs[i];
                        auto &part = result->parts[i];
                        switch (sec.variant) {
                            case ldcad_snap_meta::CylShapeVariant::L_:
                                part.type = result->parts[i + 1].type;
                                part.flexibleRadius = true;
                                break;
                            case ldcad_snap_meta::CylShapeVariant::_L:
                                part.type = result->parts[i - 1].type;
                                part.flexibleRadius = true;
                                break;
                            default:
                                break;
                        }
                    }

                    switch (cylCommand->caps) {
                        case ldcad_snap_meta::CylCaps::NONE:
                            result->openStart = true;
                            result->openEnd = true;
                            break;
                        case ldcad_snap_meta::CylCaps::ONE:
                            result->openStart = result->gender == Gender::F;
                            result->openEnd = result->gender == Gender::M;
                            break;
                        case ldcad_snap_meta::CylCaps::TWO:
                            result->openStart = false;
                            result->openEnd = false;
                            break;
                        case ldcad_snap_meta::CylCaps::A:
                            result->openStart = false;
                            result->openEnd = true;
                            break;
                        case ldcad_snap_meta::CylCaps::B:
                            result->openStart = true;
                            result->openEnd = false;
                            break;
                    }

                    //TODO center and mirror

                    if (cylCommand->grid.has_value()) {
                        std::vector<std::shared_ptr<Connector>> base{result};
                        const auto& grid = *cylCommand->grid;
                        multiplyConnectorByGrid(connectors, base, grid);
                    } else {
                        connectors.push_back(result);
                    }
                }

                const auto clpCommand = std::dynamic_pointer_cast<ldcad_snap_meta::ClpCommand>(command);
                if (clpCommand != nullptr && (!clpCommand->id.has_value() || !clearIDs.contains(*clpCommand->id))) {
                    connectors.push_back(
                            std::make_shared<ClipConnector>(
                                    "",
                                    combinePosOri(clpCommand) * transformation,
                                    clpCommand->radius,
                                    clpCommand->length,
                                    clpCommand->slide));
                }
            }

            if (!clearAll) {
                for (const auto& item: file->elements) {
                    if (item->getType() == 1) {
                        const auto sfReference = std::dynamic_pointer_cast<ldr::SubfileReference>(item);
                        createConnectors(connectors, sfReference->getFile(), sfReference->getTransformationMatrix() * transformation, clearIDs);
                    }
                }
            }
        }

        void multiplyConnectorByGrid(std::vector<std::shared_ptr<Connector>>& connectors, std::vector<std::shared_ptr<Connector>>& base, const ldcad_snap_meta::Grid& grid) {
            float xStart = grid.centerX ? (grid.countX / -2.f) * grid.spacingX : 0;
            float zStart = grid.centerZ ? (grid.countZ / -2.f) * grid.spacingZ : 0;
            for (int ix = 0; ix < grid.countX; ++ix) {
                for (int iz = 0; iz < grid.countZ; ++iz) {
                    float dx = xStart + ix * grid.spacingX;
                    float dz = zStart + iz * grid.spacingZ;
                    for (auto cn: base) {
                        cn->location = glm::translate(cn->location, {dx, 0, dz});
                        connectors.push_back(cn);
                    }
                }
            }
        }

    }
    const std::vector<std::shared_ptr<Connector>>& getConnectorsOfPart(const std::string& name) {
        const auto it = cache.find(name);
        if (it != cache.end()) {
            return it->second;
        }
        auto& result = cache.insert({name, std::vector<std::shared_ptr<Connector>>()}).first->second;
        const auto file = ldr::file_repo::get().getFile(name);
        createConnectors(result, file, glm::mat4(1.f), {});
        return result;
    }
}
