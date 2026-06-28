#include "gltf.h"

#include <core/assert.h>

#include <iostream>
#include <simdjson.h>

namespace raftel::parsers::model {

GLTFTransform::GLTFTransform(GLTFTransformComponents components)
    : componentTransform(components)
    , isMatrixTransform(false)
{
}

GLTFTransform::GLTFTransform(GLTFTransformMatrix matrix)
    : matrixTransform(matrix)
    , isMatrixTransform(true)
{
}

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

GLTFTransform GLTFParser::parseTransform(simdjson::ondemand::object node)
{
    auto matrixField = node.find_field("matrix");
    auto translationField = node.find_field("translation");
    auto rotationField = node.find_field("rotation");
    auto scaleField = node.find_field("scale");

    if (matrixField.has_value()) {
        auto matrixArray = matrixField.get_array();
        ASSERT(matrixArray.count_elements() == (4 * 4));

        return GLTFTransformMatrix {
            .modelMatrix = glm::mat4x4(
                // first row
                (float)matrixArray.at(0).get_double(),
                (float)matrixArray.at(1).get_double(),
                (float)matrixArray.at(2).get_double(),
                (float)matrixArray.at(3).get_double(),
                // second row
                (float)matrixArray.at(4).get_double(),
                (float)matrixArray.at(5).get_double(),
                (float)matrixArray.at(6).get_double(),
                (float)matrixArray.at(7).get_double(),
                // third row
                (float)matrixArray.at(8).get_double(),
                (float)matrixArray.at(9).get_double(),
                (float)matrixArray.at(10).get_double(),
                (float)matrixArray.at(11).get_double(),
                // fourth row
                (float)matrixArray.at(12).get_double(),
                (float)matrixArray.at(13).get_double(),
                (float)matrixArray.at(14).get_double(),
                (float)matrixArray.at(15).get_double()),
        };
    } else {
        glm::vec3 translation = glm::vec3(0.0f);
        glm::vec4 rotation = glm::vec4(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);

        if (translationField.has_value()) {
            auto translationArray = translationField.get_array();
            ASSERT(translationArray.count_elements() == 3);

            translation.x = (float)translationArray.at(0).get_double();
            translation.y = (float)translationArray.at(1).get_double();
            translation.z = (float)translationArray.at(2).get_double();
        }
        if (rotationField.has_value()) {
            auto rotationArray = rotationField.get_array();
            ASSERT(rotationArray.count_elements() == 4);

            rotation[0] = (float)rotationArray.at(0).get_double();
            rotation[1] = (float)rotationArray.at(1).get_double();
            rotation[2] = (float)rotationArray.at(2).get_double();
            rotation[3] = (float)rotationArray.at(3).get_double();
        }
        if (scaleField.has_value()) {
            auto scaleArray = scaleField.get_array();
            ASSERT(scaleArray.count_elements() == 3);

            scale.x = (float)scaleArray.at(0).get_double();
            scale.y = (float)scaleArray.at(1).get_double();
            scale.z = (float)scaleArray.at(2).get_double();
        }

        return GLTFTransformComponents {
            .rotation = rotation,
            .translation = translation,
            .scale = scale,
        };
    }
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
        std::cout << nodeName << std::endl;

        const auto meshField = node->find_field("mesh");
        const auto cameraField = node->find_field("camera");
        const auto childListField = node->find_field("children");

        if (meshField.has_value()) {
            result.push_back(GLTFMeshNode {
                .transform = parseTransform(node),
                .meshID = 0,
            });
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
