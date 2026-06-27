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

    const auto defaultScene = gltf["scene"].get_uint64().value();
    auto sceneNodes = gltf["scenes"]->get_array().at(defaultScene)["nodes"].get_array();
    auto nodeList = std::move(gltf["nodes"]->get_array().take_value());

    result.sceneNodes = parseNodeList(nodeList);

    return result;
}

std::vector<GLTFNode> GLTFParser::parseNodeList(simdjson::ondemand::array& nodeList)
{
    const size_t nodeCount = nodeList.count_elements().take_value();
    std::vector<GLTFNode> result;
    result.reserve(nodeCount);

    // NOTE: strings are owned by the parser, here we only take a string view into those.
    // As such for any important strings we will need to make copies. I assume that this
    // will generally speaking apply to any other dynamic data that comes from the parser
    for (auto node : nodeList) {
        std::string_view nodeName = node["name"]->get_string().take_value();

        const auto meshField = node->find_field("mesh");
        const auto cameraField = node->find_field("camera");
        const auto childListField = node->find_field("children");

        if (meshField.has_value()) {
            // mesh node
        } else if (cameraField.has_value()) {
            // camera node
        } else if (childListField.has_value()) {
            // child list intermediary node
        } else {
            // proxy node
        }
    }

    return result;
}

}
