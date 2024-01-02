#include "connector_data_provider.h"
#include "../ldr/file_repo.h"
#include "connection_check.h"
#include "spdlog/fmt/chrono.h"
#include "spdlog/spdlog.h"
#include "spdlog/stopwatch.h"

namespace bricksim::connection {
    namespace {
        uomap_t<std::shared_ptr<ldr::FileNamespace>, uomap_t<std::string, std::shared_ptr<connector_container_t>>> cache;
        std::mutex cacheLock;
    }

    void removeConnected(connector_container_t& connectors) {
        VectorPairCheckResultConsumer result;
        ConnectionCheck connCheck(result);
        connCheck.checkForConnected(connectors);
        uoset_t<std::shared_ptr<Connector>> fullyConnectedSet;
        fullyConnectedSet.reserve(result.getResult().size());
        for (const auto& item: result.getResult()) {
            if (item.completelyUsedConnector[0]) {
                fullyConnectedSet.insert(item.connectorA);
            }
            if (item.completelyUsedConnector[1]) {
                fullyConnectedSet.insert(item.connectorB);
            }
            //todo if the connector is only partially used, create a new connector of the part which is still free
            // but this will be a complex algorithm
        }
        connectors.erase(std::remove_if(connectors.begin(),
                                        connectors.end(),
                                        [&fullyConnectedSet](const auto c) {
                                            return fullyConnectedSet.contains(c);
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

    std::shared_ptr<connector_container_t> getConnectorsOfLdrFile(const std::shared_ptr<ldr::FileNamespace>& fileNamespace, const std::string& name) {
        {
            std::lock_guard<std::mutex> lg(cacheLock);
            if (const auto nsCacheIt = cache.find(fileNamespace); nsCacheIt != cache.end()) {
                if (const auto it = nsCacheIt->second.find(name); it != nsCacheIt->second.end()) {
                    return it->second;
                }
            }
        }

        const auto file = ldr::file_repo::get().getFile(fileNamespace, name);
        std::shared_ptr<connector_container_t> result;
        if (file->metaInfo.type == ldr::FileType::MODEL || file->metaInfo.type == ldr::FileType::MPD_SUBFILE) {
            result = std::make_shared<connector_container_t>();
            spdlog::stopwatch sw;
            for (const auto& item: file->elements) {
                if (item->getType() == 1) {
                    const auto sfReference = std::dynamic_pointer_cast<ldr::SubfileReference>(item);
                    const auto sfReferenceTransformation = sfReference->getTransformationMatrixT();
                    const auto partResult = getConnectorsOfLdrFile(sfReference->getFile(file));
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
            ConnectorConversion conversion;
            conversion.createConnectors(file);
            const auto duplicateCount = removeDuplicates(*conversion.getResult());
            if (duplicateCount > 0) {
                spdlog::warn("Part {} {} has {} duplicate connectors", name, file->metaInfo.title, duplicateCount);
            }
            result = conversion.getResult();
        }
        {
            std::lock_guard<std::mutex> lg(cacheLock);
            cache[fileNamespace].insert({name, result});
        }
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
