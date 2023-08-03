#include "connector_data_provider.h"
#include "../helpers/geometry.h"
#include "../ldr/file_repo.h"
#include "connection_check.h"
#include "ldcad_meta/clear_command.h"
#include "ldcad_meta/clp_command.h"
#include "ldcad_meta/cyl_command.h"
#include "ldcad_meta/fgr_command.h"
#include "ldcad_meta/gen_command.h"
#include "ldcad_meta/incl_command.h"
#include "spdlog/fmt/chrono.h"
#include "spdlog/spdlog.h"
#include "spdlog/stopwatch.h"

namespace bricksim::connection {
    namespace {
        uomap_t<std::shared_ptr<ldr::FileNamespace>, uomap_t<std::string, std::shared_ptr<connector_container_t>>> cache;

        void multiplyConnectorByGrid(connector_container_t& connectors, const connector_container_t& base, const ldcad_meta::Grid& grid, const glm::mat3& orientation);

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

        void removeConnected(connector_container_t& connectors) {
            const auto connected = getConnectedConnectors(connectors);
            uoset_t<std::shared_ptr<Connector>> connectedSet;
            connectedSet.reserve(connected.size());
            for (const auto& item: connected) {
                for (const auto& c: item) {
                    connectedSet.insert(c);
                }
            }
            connectors.erase(std::remove_if(connectors.begin(),
                                            connectors.end(),
                                            [&connectedSet](const auto c) {
                                                return connectedSet.contains(c);
                                            }),
                             connectors.end());
        }

        std::size_t removeDuplicates(connector_container_t& connectors) {
            connector_container_t result;
            result.reserve(connectors.size());
            std::size_t duplicateCount = 0;
            for (const auto& c: connectors) {
                bool add = true;
                for (const auto& r: result) {
                    if (*r == *c) {
                        add = false;
                        ++duplicateCount;
                        break;
                    }
                }
                if (add) {
                    result.push_back(c);
                }
            }
            connectors.assign(result.begin(), result.end());
            return duplicateCount;
        }

        void createConnectors(connector_container_t& connectors,
                              const std::shared_ptr<ldr::File>& file,
                              glm::mat4 const& transformation,
                              uoset_t<std::string> clearIDs,
                              std::string parentSourceTrace) {
            const auto sourceTrace = parentSourceTrace.empty()
                                             ? file->metaInfo.name
                                             : parentSourceTrace + "->" + file->metaInfo.name;
            bool clearAll = false;
            for (const auto& command: file->ldcadMetas) {
                const auto clearCommand = std::dynamic_pointer_cast<ldcad_meta::ClearCommand>(command);
                if (clearCommand != nullptr) {
                    if (clearCommand->id.has_value()) {
                        clearIDs.insert(*clearCommand->id);
                    } else {
                        clearAll = true;
                        connectors.clear();
                    }
                }

                const auto inclCommand = std::dynamic_pointer_cast<ldcad_meta::InclCommand>(command);
                if (inclCommand != nullptr) {
                    glm::mat4 transf = combinePosOriScale(inclCommand);

                    const auto includedFile = ldr::file_repo::get().getFile(nullptr, inclCommand->ref);

                    if (inclCommand->grid.has_value()) {
                        connector_container_t base;
                        createConnectors(base, includedFile, transf * transformation, clearIDs, sourceTrace);
                        const auto& grid = *inclCommand->grid;
                        multiplyConnectorByGrid(connectors, base, grid, inclCommand->ori.value_or(glm::mat3(1.f)));
                    } else {
                        createConnectors(connectors, includedFile, transf * transformation, clearIDs, sourceTrace);
                    }
                }

                const auto cylCommand = std::dynamic_pointer_cast<ldcad_meta::CylCommand>(command);
                if (cylCommand != nullptr && (!cylCommand->id.has_value() || !clearIDs.contains(*cylCommand->id))) {
                    //TODO check cylCommand->scale
                    const auto cylTransf = combinePosOri(cylCommand) * transformation;

                    if (geometry::doesTransformationInverseWindingOrder(cylTransf)) {
                        if (cylCommand->mirror == ldcad_meta::MirrorType::NONE) {
                            continue;
                        } else {
                            //todo handle COR
                        }
                    }

                    auto result = std::make_shared<CylindricalConnector>(
                            cylCommand->group.value_or(""),
                            cylTransf[3],
                            cylTransf * glm::vec4(0.f, -1.f, 0.f, 0.f) /*cylCommand->ori.value_or(glm::mat3(1.f))*glm::vec3(0.f, -1.f, 0.f)*/,
                            sourceTrace,
                            cylCommand->gender == ldcad_meta::Gender::M
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
                    for (size_t i = 0; i < cylCommand->secs.size(); ++i) {
                        const auto& sec = cylCommand->secs[i];
                        auto& part = result->parts[i];
                        switch (sec.variant) {
                            case ldcad_meta::CylShapeVariant::R:
                                part.type = CylindricalShapeType::ROUND;
                                break;
                            case ldcad_meta::CylShapeVariant::A:
                                part.type = CylindricalShapeType::AXLE;
                                break;
                            case ldcad_meta::CylShapeVariant::S:
                                part.type = CylindricalShapeType::SQUARE;
                                break;
                            default:
                                break;
                        }
                    }
                    for (size_t i = 0; i < cylCommand->secs.size(); ++i) {
                        const auto& sec = cylCommand->secs[i];
                        auto& part = result->parts[i];
                        switch (sec.variant) {
                            case ldcad_meta::CylShapeVariant::L_:
                                part.type = result->parts[i + 1].type;
                                part.flexibleRadius = true;
                                break;
                            case ldcad_meta::CylShapeVariant::_L:
                                part.type = result->parts[i - 1].type;
                                part.flexibleRadius = true;
                                break;
                            default:
                                break;
                        }
                    }

                    switch (cylCommand->caps) {
                        case ldcad_meta::CylCaps::NONE:
                            result->openStart = true;
                            result->openEnd = true;
                            break;
                        case ldcad_meta::CylCaps::ONE:
                            result->openStart = result->gender == Gender::F;
                            result->openEnd = result->gender == Gender::M;
                            break;
                        case ldcad_meta::CylCaps::TWO:
                            result->openStart = false;
                            result->openEnd = false;
                            break;
                        case ldcad_meta::CylCaps::A:
                            result->openStart = false;
                            result->openEnd = true;
                            break;
                        case ldcad_meta::CylCaps::B:
                            result->openStart = true;
                            result->openEnd = false;
                            break;
                    }

                    //TODO mirror
                    if (cylCommand->center) {
                        result->start += result->direction * (result->getTotalLength() * -.5f);
                    }

                    if (cylCommand->grid.has_value()) {
                        connector_container_t base{result};
                        const auto& grid = *cylCommand->grid;
                        multiplyConnectorByGrid(connectors, base, grid, cylCommand->ori.value_or(glm::mat3(1.f)));
                    } else {
                        connectors.push_back(result);
                    }
                }

                const auto clpCommand = std::dynamic_pointer_cast<ldcad_meta::ClpCommand>(command);
                if (clpCommand != nullptr && (!clpCommand->id.has_value() || !clearIDs.contains(*clpCommand->id))) {
                    const auto clpTransf = combinePosOri(clpCommand) * transformation;
                    connectors.push_back(
                            std::make_shared<ClipConnector>(
                                    "",
                                    clpTransf * glm::vec4(0.f, 0.f, 0.f, 1.f),
                                    clpTransf * glm::vec4(0.f, -1.f, 0.f, 0.f),
                                    sourceTrace,
                                    clpCommand->radius,
                                    clpCommand->length,
                                    clpCommand->slide));
                }

                const auto fgrCommand = std::dynamic_pointer_cast<ldcad_meta::FgrCommand>(command);
                if (fgrCommand != nullptr && (!fgrCommand->id.has_value() || !clearIDs.contains(*fgrCommand->id))) {
                    const auto fgrTransf = combinePosOri(fgrCommand) * transformation;
                    connectors.push_back(
                            std::make_shared<FingerConnector>(
                                    fgrCommand->group.value_or(""),
                                    fgrTransf * glm::vec4(0.f, 0.f, 0.f, 1.f),
                                    fgrTransf * glm::vec4(0.f, -1.f, 0.f, 0.f),
                                    sourceTrace,
                                    fgrCommand->genderOfs == ldcad_meta::Gender::M
                                            ? Gender::M
                                            : Gender::F,
                                    fgrCommand->radius,
                                    fgrCommand->seq));
                }

                const auto genCommand = std::dynamic_pointer_cast<ldcad_meta::GenCommand>(command);
                if (genCommand != nullptr && (!genCommand->id.has_value() || !clearIDs.contains(*genCommand->id))) {
                    const auto genTransf = combinePosOri(genCommand) * transformation;
                    connectors.push_back(
                            std::make_shared<GenericConnector>(
                                    genCommand->group.value_or(""),
                                    genTransf * glm::vec4(0.f, 0.f, 0.f, 1.f),
                                    genTransf * glm::vec4(0.f, -1.f, 0.f, 0.f),
                                    sourceTrace,
                                    genCommand->gender == ldcad_meta::Gender::M
                                            ? Gender::M
                                            : Gender::F,
                                    genCommand->bounding));
                }
            }

            if (!clearAll) {
                for (const auto& item: file->elements) {
                    if (item->getType() == 1) {
                        const auto sfReference = std::dynamic_pointer_cast<ldr::SubfileReference>(item);
                        const auto sfReferenceTransformation = sfReference->getTransformationMatrix();
                        createConnectors(connectors, sfReference->getFile(file->nameSpace), transformation * sfReferenceTransformation, clearIDs, sourceTrace);
                    }
                }
            }
        }

        void multiplyConnectorByGrid(connector_container_t& connectors,
                                     const connector_container_t& base,
                                     const ldcad_meta::Grid& grid,
                                     const glm::mat3& orientation) {
            float xStart = grid.centerX ? (static_cast<float>(grid.countX - 1) / -2.f) * grid.spacingX : 0;
            float zStart = grid.centerZ ? (static_cast<float>(grid.countZ - 1) / -2.f) * grid.spacingZ : 0;
            for (uint32_t ix = 0; ix < grid.countX; ++ix) {
                for (uint32_t iz = 0; iz < grid.countZ; ++iz) {
                    float dx = xStart + static_cast<float>(ix) * grid.spacingX;
                    float dz = zStart + static_cast<float>(iz) * grid.spacingZ;
                    for (const auto& cn: base) {
                        auto clone = cn->clone();
                        clone->start += (glm::vec3(dx, 0, dz) * orientation);
                        connectors.push_back(clone);
                    }
                }
            }
        }
    }
    std::shared_ptr<connector_container_t> getConnectorsOfLdrFile(const std::shared_ptr<ldr::FileNamespace>& fileNamespace, const std::string& name) {
        if (const auto nsCacheIt = cache.find(fileNamespace); nsCacheIt != cache.end()) {
            if (const auto it = nsCacheIt->second.find(name); it != nsCacheIt->second.end()) {
                return it->second;
            }
        }

        const auto file = ldr::file_repo::get().getFile(fileNamespace, name);
        auto result = std::make_shared<connector_container_t>();
        if (file->metaInfo.type == ldr::FileType::MODEL || file->metaInfo.type == ldr::FileType::MPD_SUBFILE) {
            spdlog::stopwatch sw;
            for (const auto& item: file->elements) {
                if (item->getType() == 1) {
                    const auto sfReference = std::dynamic_pointer_cast<ldr::SubfileReference>(item);
                    const auto sfReferenceTransformation = sfReference->getTransformationMatrixT();
                    const auto partResult = getConnectorsOfLdrFile(sfReference->getFile(file->nameSpace));
                    for (const auto& partConn: *partResult) {
                        auto transfConn = partConn->transform(sfReferenceTransformation);
                        transfConn->sourceTrace = file->metaInfo.name + "->" + transfConn->sourceTrace;
                        result->push_back(transfConn);
                    }
                }
            }
            removeConnected(*result);
            if (result->size() > 1000) {
                const auto time = std::chrono::duration_cast<std::chrono::milliseconds>(sw.elapsed());
                spdlog::debug("connector data provider: provided {} connectors for {} in {}", result->size(), name, time);
            }
        } else {
            createConnectors(*result, file, glm::mat4(1.f), {}, "");
            const auto duplicateCount = removeDuplicates(*result);
            if (duplicateCount > 0) {
                spdlog::warn("Part {} {} has {} duplicate connectors", name, file->metaInfo.title, duplicateCount);
            }
        }
        cache[fileNamespace].insert({name, result});
        return result;
    }
    std::shared_ptr<connector_container_t> getConnectorsOfNode(const std::shared_ptr<etree::MeshNode>& node) {
        if (node->getType() == etree::NodeType::TYPE_PART) {
            return getConnectorsOfLdrFile(std::dynamic_pointer_cast<etree::LdrNode>(node)->ldrFile);
        } else if (node->getType() == etree::NodeType::TYPE_MODEL_INSTANCE) {
            return getConnectorsOfLdrFile(std::dynamic_pointer_cast<etree::ModelInstanceNode>(node)->modelNode->ldrFile);
        }
        static const auto empty = std::make_shared<connector_container_t>();
        return empty;
    }
    std::shared_ptr<connector_container_t> getConnectorsOfLdrFile(const std::shared_ptr<ldr::File>& ldrFile) {
        return getConnectorsOfLdrFile(ldrFile->nameSpace, ldrFile->metaInfo.name);
    }
}
