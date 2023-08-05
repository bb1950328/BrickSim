#include "connector_conversion.h"
#include "../helpers/geometry.h"
#include "../ldr/file_repo.h"
#include <numeric>

namespace bricksim::connection {
    namespace {
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
    }

    void ConnectorConversion::addConnectorsWithGrid(const connector_container_t& base, const ldcad_meta::Grid& grid, const glm::mat3& orientation) {
        float xStart = grid.centerX ? (static_cast<float>(grid.countX - 1) / -2.f) * grid.spacingX : 0;
        float zStart = grid.centerZ ? (static_cast<float>(grid.countZ - 1) / -2.f) * grid.spacingZ : 0;
        for (uint32_t ix = 0; ix < grid.countX; ++ix) {
            for (uint32_t iz = 0; iz < grid.countZ; ++iz) {
                float dx = xStart + static_cast<float>(ix) * grid.spacingX;
                float dz = zStart + static_cast<float>(iz) * grid.spacingZ;
                for (const auto& cn: base) {
                    auto clone = cn->clone();
                    clone->start += (orientation * glm::vec3(dx, 0, dz));
                    result->push_back(clone);
                }
            }
        }
    }
    void ConnectorConversion::createConnectors(const std::shared_ptr<ldr::File>& file, const glm::mat4& transformation, std::string parentSourceTrace) {
        const auto sourceTrace = parentSourceTrace.empty()
                                         ? file->metaInfo.name
                                         : parentSourceTrace + "->" + file->metaInfo.name;
        for (const auto& command: file->ldcadMetas) {
            switch (command->type) {
                case ldcad_meta::CommandType::SNAP_CLEAR:
                    handleClearCommand(std::dynamic_pointer_cast<ldcad_meta::ClearCommand>(command));
                    break;
                case ldcad_meta::CommandType::SNAP_INCL:
                    convertInclCommand(transformation, sourceTrace, std::dynamic_pointer_cast<ldcad_meta::InclCommand>(command));
                    break;
                case ldcad_meta::CommandType::SNAP_CYL: {
                    const auto cylCommand = std::dynamic_pointer_cast<ldcad_meta::CylCommand>(command);
                    if (idNotCleared(cylCommand->id)) {
                        convertCylCommand(transformation, sourceTrace, cylCommand);
                    }
                    break;
                }
                case ldcad_meta::CommandType::SNAP_CLP: {
                    const auto clpCommand = std::dynamic_pointer_cast<ldcad_meta::ClpCommand>(command);
                    if (idNotCleared(clpCommand->id)) {
                        convertClpCommand(transformation, sourceTrace, clpCommand);
                    }
                    break;
                }
                case ldcad_meta::CommandType::SNAP_FGR: {
                    const auto fgrCommand = std::dynamic_pointer_cast<ldcad_meta::FgrCommand>(command);
                    if (idNotCleared(fgrCommand->id)) {
                        convertFgrCommand(transformation, sourceTrace, fgrCommand);
                    }
                    break;
                }
                case ldcad_meta::CommandType::SNAP_GEN: {
                    const auto genCommand = std::dynamic_pointer_cast<ldcad_meta::GenCommand>(command);
                    if (idNotCleared(genCommand->id)) {
                        convertGenCommand(transformation, sourceTrace, genCommand);
                    }
                    break;
                }
                default: break;
            }
        }

        if (includeSubfileReferences) {
            for (const auto& item: file->elements) {
                if (item->getType() == 1) {
                    convertSubfileReference(transformation, sourceTrace, file->nameSpace, std::dynamic_pointer_cast<ldr::SubfileReference>(item));
                }
            }
        }
    }
    void ConnectorConversion::convertInclCommand(const glm::mat4& transformation, const std::string& sourceTrace, const std::shared_ptr<ldcad_meta::InclCommand>& command) {
        glm::mat4 transf = combinePosOriScale(command);

        const auto includedFile = ldr::file_repo::get().getFile(nullptr, command->ref);

        if (command->grid.has_value()) {
            ConnectorConversion inclConversion;
            inclConversion.createConnectors(includedFile, transf * transformation, sourceTrace);
            addConnectorsWithGrid(*inclConversion.getResult(), *command->grid, command->ori.value_or(glm::mat3(1.f)));
        } else {
            createConnectors(includedFile, transf * transformation, sourceTrace);
        }
    }
    void ConnectorConversion::handleClearCommand(const std::shared_ptr<ldcad_meta::ClearCommand>& command) {
        if (command->id.has_value()) {
            clearIDs.insert(*command->id);
        } else {
            includeSubfileReferences = false;
            result->clear();
        }
    }
    void ConnectorConversion::convertCylCommand(const glm::mat4& transformation, const std::string& sourceTrace, const std::shared_ptr<ldcad_meta::CylCommand>& command) {
        //TODO check cylCommand->scale
        auto cylTransf = transformation * combinePosOri(command);

        if (geometry::doesTransformationInverseWindingOrder(cylTransf)) {
            if (command->mirror == ldcad_meta::MirrorType::NONE) {
                return;
            } else {
                //todo handle COR
                // documentation is a bit unclear
            }
        }

        const glm::vec3 direction = cylTransf * glm::vec4(0.f, -1.f, 0.f, 0.f);
        const float lengthScale = glm::length(direction);
        const float radiusScale = glm::length(glm::vec3(cylTransf * glm::vec4(1.f, 0.f, 0.f, 0.f)));
        std::vector<CylindricalShapePart> shapeParts;
        shapeParts.reserve(command->secs.size());
        for (const auto& sec: command->secs) {
            shapeParts.push_back({
                    CylindricalShapeType::ROUND,
                    false,
                    sec.radius * radiusScale,
                    sec.length * lengthScale,
            });
        }
        auto connector = std::make_shared<CylindricalConnector>(
                command->group.value_or(""),
                cylTransf[3],
                direction, /*cylCommand->ori.value_or(glm::mat3(1.f))*glm::vec3(0.f, -1.f, 0.f)*/
                sourceTrace,
                command->gender == ldcad_meta::Gender::M
                        ? Gender::M
                        : Gender::F,
                shapeParts,
                false,
                false,
                command->slide);

        for (size_t i = 0; i < command->secs.size(); ++i) {
            const auto& sec = command->secs[i];
            auto& part = connector->parts[i];
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
        for (size_t i = 0; i < command->secs.size(); ++i) {
            const auto& sec = command->secs[i];
            auto& part = connector->parts[i];
            switch (sec.variant) {
                case ldcad_meta::CylShapeVariant::L_:
                    part.type = connector->parts[i + 1].type;
                    part.flexibleRadius = true;
                    break;
                case ldcad_meta::CylShapeVariant::_L:
                    part.type = connector->parts[i - 1].type;
                    part.flexibleRadius = true;
                    break;
                default:
                    break;
            }
        }

        switch (command->caps) {
            case ldcad_meta::CylCaps::NONE:
                connector->openStart = true;
                connector->openEnd = true;
                break;
            case ldcad_meta::CylCaps::ONE:
                connector->openStart = connector->gender == Gender::F;
                connector->openEnd = connector->gender == Gender::M;
                break;
            case ldcad_meta::CylCaps::TWO:
                connector->openStart = false;
                connector->openEnd = false;
                break;
            case ldcad_meta::CylCaps::A:
                connector->openStart = false;
                connector->openEnd = true;
                break;
            case ldcad_meta::CylCaps::B:
                connector->openStart = true;
                connector->openEnd = false;
                break;
        }

        //TODO mirror
        if (command->center) {
            connector->start += connector->direction * (connector->totalLength * -.5f);
        }

        if (command->grid.has_value()) {
            connector_container_t base{connector};
            const auto& grid = *command->grid;
            addConnectorsWithGrid(base, grid, command->ori.value_or(glm::mat3(1.f)));
        } else {
            this->result->push_back(connector);
        }
    }
    void ConnectorConversion::convertClpCommand(const glm::mat4& transformation, const std::string& sourceTrace, const std::shared_ptr<ldcad_meta::ClpCommand>& command) {
        const auto clpTransf = transformation * combinePosOri(command);
        const glm::vec3 direction = clpTransf * glm::vec4(0.f, -1.f, 0.f, 0.f);
        const glm::vec3 openingDirection = clpTransf * glm::vec4(0.f, 0.f, -1.f, 0.f);
        glm::vec3 start = clpTransf[3];
        if (command->center) {
            start -= direction * command->length * .5f;
        }
        result->push_back(
                std::make_shared<ClipConnector>(
                        "",
                        start,
                        direction,
                        sourceTrace,
                        command->radius,
                        command->length,
                        command->slide,
                        openingDirection));
    }
    void ConnectorConversion::convertFgrCommand(const glm::mat4& transformation, const std::string& sourceTrace, const std::shared_ptr<ldcad_meta::FgrCommand>& command) {
        const auto fgrTransf = transformation * combinePosOri(command);
        const glm::vec3 direction = fgrTransf * glm::vec4(0.f, -1.f, 0.f, 0.f);
        glm::vec3 start = fgrTransf[3];
        if (command->center) {
            const auto totalLength = std::accumulate(command->seq.cbegin(), command->seq.cend(), 0.f);
            start -= direction * totalLength * .5f;
        }
        result->push_back(
                std::make_shared<FingerConnector>(
                        command->group.value_or(""),
                        start,
                        direction,
                        sourceTrace,
                        command->genderOfs == ldcad_meta::Gender::M
                                ? Gender::M
                                : Gender::F,
                        command->radius,
                        command->seq));
    }
    void ConnectorConversion::convertGenCommand(const glm::mat4& transformation, const std::string& sourceTrace, const std::shared_ptr<ldcad_meta::GenCommand>& command) {
        const auto genTransf = combinePosOri(command) * transformation;
        result->push_back(
                std::make_shared<GenericConnector>(
                        command->group.value_or(""),
                        genTransf * glm::vec4(0.f, 0.f, 0.f, 1.f),
                        genTransf * glm::vec4(1.f, 0.f, 0.f, 0.f),
                        sourceTrace,
                        command->gender == ldcad_meta::Gender::M
                                ? Gender::M
                                : Gender::F,
                        command->bounding));
    }
    void ConnectorConversion::convertSubfileReference(const glm::mat4& transformation, const std::string& sourceTrace, const std::shared_ptr<ldr::FileNamespace>& fileNamespace, const std::shared_ptr<ldr::SubfileReference>& sfReference) {
        const auto sfReferenceTransformation = sfReference->getTransformationMatrix();
        const auto referencedFile = sfReference->getFile(fileNamespace);
        const auto combinedTransformation = transformation * sfReferenceTransformation;
        if (referencedFile->metaInfo.type == ldr::FileType::PRIMITIVE
            || referencedFile->metaInfo.type == ldr::FileType::SUBPART) {
            createConnectors(referencedFile, combinedTransformation, sourceTrace);
        } else {
            //use temporary container so clear commands do not wipe the entire hierarchy
            ConnectorConversion referencedPartConversion;
            referencedPartConversion.createConnectors(referencedFile, combinedTransformation, sourceTrace);
            result->insert(result->end(),
                           referencedPartConversion.getResult()->cbegin(),
                           referencedPartConversion.getResult()->cend());
        }
    }
    ConnectorConversion::ConnectorConversion() :
        result(std::make_shared<connector_container_t>()) {}
    const std::shared_ptr<connector_container_t>& ConnectorConversion::getResult() const {
        return result;
    }
    void ConnectorConversion::createConnectors(const std::shared_ptr<ldr::File>& file) {
        createConnectors(file, glm::mat4(1.f), "");
    }
    bool ConnectorConversion::idNotCleared(const std::optional<std::string>& id) const {
        return !id.has_value() || !clearIDs.contains(*id);
    }
}
