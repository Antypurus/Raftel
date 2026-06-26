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

    return result;
}

}
