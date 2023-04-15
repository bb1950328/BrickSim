#include "connector_data_provider.h"
#include "../helpers/geometry.h"
#include "../ldr/file_repo.h"
#include "ldcad_snap_meta/clear_command.h"
#include "ldcad_snap_meta/clp_command.h"
#include "ldcad_snap_meta/cyl_command.h"
#include "ldcad_snap_meta/fgr_command.h"
#include "ldcad_snap_meta/gen_command.h"
#include "ldcad_snap_meta/incl_command.h"
#include "spdlog/spdlog.h"
namespace bricksim::connection {
    namespace {
        uomap_t<std::string, std::vector<std::shared_ptr<Connector>>> cache;

        void multiplyConnectorByGrid(std::vector<std::shared_ptr<Connector>>& connectors, const std::vector<std::shared_ptr<Connector>>& base, const ldcad_snap_meta::Grid& grid);

        template<typename T>
        [[nodiscard]] glm::mat4 combinePosOri(const std::shared_ptr<T>& command) {
            glm::mat4 transf(1.f);
            if (command->pos.has_value()) {
                transf[3] = glm::vec4(*command->pos, 1.f);
            }
            if (command->ori.has_value()) {
                for (int x = 0; x < 3; ++x) {
                    for (int y = 0; y < 3; ++y) {
                        transf[x][y] = (*command->ori)[x][y];
                    }
                }
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

        void createConnectors(std::vector<std::shared_ptr<Connector>>& connectors,
                              const std::shared_ptr<ldr::File>& file,
                              glm::mat4 const& transformation,
                              uoset_t<std::string> clearIDs) {
            if (file->metaInfo.type == ldr::FileType::MODEL) {
                return;
            }

            bool clearAll = false;
            for (const auto& command: file->ldcadSnapMetas) {
                const auto clearCommand = std::dynamic_pointer_cast<ldcad_snap_meta::ClearCommand>(command);
                if (clearCommand != nullptr) {
                    if (clearCommand->id.has_value()) {
                        clearIDs.insert(*clearCommand->id);
                    } else {
                        clearAll = true;
                        connectors.clear();
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
                    const auto cylTransf = combinePosOri(cylCommand) * transformation;

                    if (geometry::doesTransformationInverseWindingOrder(cylTransf)) {
                        if (cylCommand->mirror == ldcad_snap_meta::MirrorType::NONE) {
                            continue;
                        } else {
                            //todo handle COR
                        }
                    }

                    auto result = std::make_shared<CylindricalConnector>(
                            cylCommand->group.value_or(""),
                            cylTransf[3],
                            cylTransf * glm::vec4(0.f, -1.f, 0.f, 0.f) /*cylCommand->ori.value_or(glm::mat3(1.f))*glm::vec3(0.f, -1.f, 0.f)*/,
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
                        const auto& sec = cylCommand->secs[i];
                        auto& part = result->parts[i];
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

                    //TODO mirror
                    if (cylCommand->center) {
                        result->start += result->direction * (result->getTotalLength() * -.5f);
                    }

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
                    const auto clpTransf = combinePosOri(clpCommand) * transformation;
                    connectors.push_back(
                            std::make_shared<ClipConnector>(
                                    "",
                                    clpTransf * glm::vec4(0.f, 0.f, 0.f, 1.f),
                                    clpTransf * glm::vec4(0.f, -1.f, 0.f, 0.f),
                                    clpCommand->radius,
                                    clpCommand->length,
                                    clpCommand->slide));
                }

                const auto fgrCommand = std::dynamic_pointer_cast<ldcad_snap_meta::FgrCommand>(command);
                if (fgrCommand != nullptr && (!fgrCommand->id.has_value() || !clearIDs.contains(*fgrCommand->id))) {
                    const auto fgrTransf = combinePosOri(fgrCommand) * transformation;
                    connectors.push_back(
                            std::make_shared<FingerConnector>(
                                    fgrCommand->group.value_or(""),
                                    fgrTransf * glm::vec4(0.f, 0.f, 0.f, 1.f),
                                    fgrTransf * glm::vec4(0.f, -1.f, 0.f, 0.f),
                                    fgrCommand->genderOfs == ldcad_snap_meta::Gender::M
                                            ? Gender::M
                                            : Gender::F,
                                    fgrCommand->radius,
                                    fgrCommand->seq));
                }

                const auto genCommand = std::dynamic_pointer_cast<ldcad_snap_meta::GenCommand>(command);
                if (genCommand != nullptr && (!genCommand->id.has_value() || !clearIDs.contains(*genCommand->id))) {
                    const auto genTransf = combinePosOri(genCommand) * transformation;
                    connectors.push_back(
                            std::make_shared<GenericConnector>(
                                    genCommand->group.value_or(""),
                                    genTransf * glm::vec4(0.f, 0.f, 0.f, 1.f),
                                    genTransf * glm::vec4(0.f, -1.f, 0.f, 0.f),
                                    genCommand->gender == ldcad_snap_meta::Gender::M
                                            ? Gender::M
                                            : Gender::F,
                                    genCommand->bounding));
                }
            }

            if (!clearAll) {
                for (const auto& item: file->elements) {
                    if (item->getType() == 1) {
                        const auto sfReference = std::dynamic_pointer_cast<ldr::SubfileReference>(item);
                        const auto sfReferenceTransformation = glm::transpose(sfReference->getTransformationMatrix());
                        createConnectors(connectors, sfReference->getFile(), sfReferenceTransformation * transformation, clearIDs);
                    }
                }
            }
        }

        void multiplyConnectorByGrid(std::vector<std::shared_ptr<Connector>>& connectors,
                                     const std::vector<std::shared_ptr<Connector>>& base,
                                     const ldcad_snap_meta::Grid& grid) {
            float xStart = grid.centerX ? (static_cast<float>(grid.countX - 1) / -2.f) * grid.spacingX : 0;
            float zStart = grid.centerZ ? (static_cast<float>(grid.countZ - 1) / -2.f) * grid.spacingZ : 0;
            for (int ix = 0; ix < grid.countX; ++ix) {
                for (int iz = 0; iz < grid.countZ; ++iz) {
                    float dx = xStart + static_cast<float>(ix) * grid.spacingX;
                    float dz = zStart + static_cast<float>(iz) * grid.spacingZ;
                    for (const auto& cn: base) {
                        auto clone = cn->clone();
                        clone->start += glm::vec3(dx, 0, dz);
                        connectors.push_back(clone);
                    }
                }
            }
        }

    }
    const std::vector<std::shared_ptr<Connector>>& getConnectorsOfPart(const std::string& name) {
        if (const auto it = cache.find(name); it != cache.end()) {
            return it->second;
        }
        auto& result = cache.insert({name, std::vector<std::shared_ptr<Connector>>()}).first->second;
        const auto file = ldr::file_repo::get().getFile(name);
        createConnectors(result, file, glm::mat4(1.f), {});
        return result;
    }
}
