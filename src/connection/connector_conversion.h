#pragma once
#include "../ldr/files.h"
#include "connection.h"
#include "ldcad_meta/clear_command.h"
#include "ldcad_meta/clp_command.h"
#include "ldcad_meta/cyl_command.h"
#include "ldcad_meta/fgr_command.h"
#include "ldcad_meta/gen_command.h"
#include "ldcad_meta/incl_command.h"
namespace bricksim::connection {
    using connector_container_t = std::vector<std::shared_ptr<Connector>>;
    class ConnectorConversion {
    protected:
        std::shared_ptr<connector_container_t> result;
        uoset_t<std::string> clearIDs;
        bool includeSubfileReferences = true;

        void addConnectorsWithGrid(const connector_container_t& base, const ldcad_meta::Grid& grid, const glm::mat3& orientation);
        void handleClearCommand(const std::shared_ptr<ldcad_meta::ClearCommand>& command);
        void convertInclCommand(const glm::mat4& transformation, const std::string& sourceTrace, const std::shared_ptr<ldcad_meta::InclCommand>& command);
        void convertCylCommand(const glm::mat4& transformation, const std::string& sourceTrace, const std::shared_ptr<ldcad_meta::CylCommand>& command);
        void convertClpCommand(const glm::mat4& transformation, const std::string& sourceTrace, const std::shared_ptr<ldcad_meta::ClpCommand>& command);
        void convertFgrCommand(const glm::mat4& transformation, const std::string& sourceTrace, const std::shared_ptr<ldcad_meta::FgrCommand>& command);
        void convertGenCommand(const glm::mat4& transformation, const std::string& sourceTrace, const std::shared_ptr<ldcad_meta::GenCommand>& command);
        void convertSubfileReference(const glm::mat4& transformation,
                                     const std::string& sourceTrace,
                                     const std::shared_ptr<ldr::File>& file,
                                     const std::shared_ptr<ldr::SubfileReference>& sfReference);
        void createConnectors(const std::shared_ptr<ldr::File>& file,
                              glm::mat4 const& transformation,
                              std::string parentSourceTrace);

        bool idNotCleared(const std::optional<std::string>& id) const;

    public:
        void createConnectors(const std::shared_ptr<ldr::File>& file);
        ConnectorConversion();
        const std::shared_ptr<connector_container_t>& getResult() const;
    };
}
