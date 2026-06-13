#include "gltf.h"

#include <iostream>
#include <simdjson.h>

namespace raftel::parsers::model {

std::optional<GLTFModel> GLTFParser::parse(std::string_view path)
{
    GLTFModel result;

    simdjson::ondemand::parser gltfParser;
    auto rawJSON = simdjson::padded_string::load(path);

    simdjson::ondemand::document gltf = gltfParser.iterate(rawJSON);

    auto sceneNodes = gltf["scenes"]->get_array().at(0)["nodes"].get_array();
    for (auto nodeID : sceneNodes) {
        result.sceneNodeIDs.push_back(uint64_t(nodeID.value()));
    }

    for (const auto& val : result.sceneNodeIDs) {
        std::cout << val << std::endl;
    }

    return result;
}

}
