#include "gltf.h"

#include <core/assert.h>
#include <core/logger.h>
#include <core/measure.h>

#include <simdjson.h>

#include <iostream>

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

GLTFNode::GLTFNode(std::uint64_t id, std::string name, GLTFProxyNode proxy)
    : id(id)
    , name(name)
    , proxyNode(proxy)
    , nodeType(GLTFNodeType::Proxy)
{
}

GLTFTransform::~GLTFTransform()
{
    if (this->isMatrixTransform) {
        this->matrixTransform.~GLTFTransformMatrix();
    } else {
        this->componentTransform.~GLTFTransformComponents();
    }
}

GLTFTransform::GLTFTransform(const GLTFTransform& other)
{
    *this = other;
}

GLTFTransform::GLTFTransform(GLTFTransform&& other)
{
    *this = other;
}

GLTFTransform& GLTFTransform::operator=(const GLTFTransform& other)
{
    if (this == &other)
        return *this;

    this->~GLTFTransform();

    this->isMatrixTransform = other.isMatrixTransform;
    if (this->isMatrixTransform) {
        new (&this->matrixTransform) GLTFTransformMatrix(other.matrixTransform);
    } else {
        new (&this->componentTransform) GLTFTransformComponents(other.componentTransform);
    }

    return *this;
}

GLTFTransform& GLTFTransform::operator=(GLTFTransform&& other)
{
    if (this == &other)
        return *this;

    this->~GLTFTransform();

    this->isMatrixTransform = other.isMatrixTransform;
    if (this->isMatrixTransform) {
        new (&this->matrixTransform) GLTFTransformMatrix(std::move(other.matrixTransform));
    } else {
        new (&this->componentTransform) GLTFTransformComponents(std::move(other.componentTransform));
    }

    return *this;
}

GLTFNode::GLTFNode(std::uint64_t id, std::string name, GLTFMeshNode mesh)
    : id(id)
    , name(name)
    , meshNode(mesh)
    , nodeType(GLTFNodeType::Mesh)
{
}

GLTFNode::GLTFNode(std::uint64_t id, std::string name, GLTFCameraNode camera)
    : id(id)
    , name(name)
    , cameraNode(camera)
    , nodeType(GLTFNodeType::Camera)
{
}

GLTFNode::GLTFNode(std::uint64_t id, std::string name, GLTFChildListNode childList)
    : id(id)
    , name(name)
    , childListNode(childList)
    , nodeType(GLTFNodeType::ChildList)
{
}

GLTFNode::GLTFNode()
    : id(0xFFFFFFFFFFFFFFFF)
    , name("")
    , proxyNode({})
    , nodeType(GLTFNodeType::Proxy)
{
}

GLTFNode::~GLTFNode()
{
    switch (this->nodeType) {
    case (GLTFNodeType::Proxy): {
        this->proxyNode.~GLTFProxyNode();
        break;
    }
    case (GLTFNodeType::Mesh): {
        this->meshNode.~GLTFMeshNode();
        break;
    }
    case (GLTFNodeType::Camera): {
        this->cameraNode.~GLTFCameraNode();
        break;
    }
    case (GLTFNodeType::ChildList): {
        this->childListNode.~GLTFChildListNode();
        break;
    }
    default:
        break;
    }
}

GLTFNode::GLTFNode(const GLTFNode& other)
    : id(other.id)
    , name(other.name)
    , nodeType(other.nodeType)
{
    switch (this->nodeType) {
    case (GLTFNodeType::Proxy): {
        new (&this->proxyNode) GLTFProxyNode(other.proxyNode);
        break;
    }
    case (GLTFNodeType::Mesh): {
        new (&this->meshNode) GLTFMeshNode(other.meshNode);
        break;
    }
    case (GLTFNodeType::Camera): {
        new (&this->cameraNode) GLTFCameraNode(other.cameraNode);
        break;
    }
    case (GLTFNodeType::ChildList): {
        new (&this->childListNode) GLTFChildListNode(other.childListNode);
        break;
    }
    default: {
        std::unreachable();
        break;
    }
    }
}

GLTFNode::GLTFNode(GLTFNode&& other)
    : id(other.id)
    , name(std::move(other.name))
    , nodeType(other.nodeType)
{
    switch (this->nodeType) {
    case (GLTFNodeType::Proxy): {
        new (&this->proxyNode) GLTFProxyNode(std::move(other.proxyNode));
        break;
    }
    case (GLTFNodeType::Mesh): {
        new (&this->meshNode) GLTFMeshNode(std::move(other.meshNode));
        break;
    }
    case (GLTFNodeType::Camera): {
        new (&this->cameraNode) GLTFCameraNode(std::move(other.cameraNode));
        break;
    }
    case (GLTFNodeType::ChildList): {
        new (&this->childListNode) GLTFChildListNode(std::move(other.childListNode));
        break;
    }
    default: {
        std::unreachable();
        break;
    }
    }
}

GLTFNode& GLTFNode::operator=(const GLTFNode& other)
{
    if (this == &other) {
        return *this;
    }
    this->~GLTFNode();

    this->id = other.id;
    this->name = other.name;
    this->nodeType = other.nodeType;
    switch (this->nodeType) {
    case (GLTFNodeType::Proxy): {
        new (&this->proxyNode) GLTFProxyNode(other.proxyNode);
        break;
    }
    case (GLTFNodeType::Mesh): {
        new (&this->meshNode) GLTFMeshNode(other.meshNode);
        break;
    }
    case (GLTFNodeType::Camera): {
        new (&this->cameraNode) GLTFCameraNode(other.cameraNode);
        break;
    }
    case (GLTFNodeType::ChildList): {
        new (&this->childListNode) GLTFChildListNode(other.childListNode);
        break;
    }
    default: {
        std::unreachable();
        break;
    }
    }

    return *this;
}

GLTFNode& GLTFNode::operator=(GLTFNode&& other)
{
    if (this == &other) {
        return *this;
    }
    this->~GLTFNode();

    this->id = other.id;
    this->name = std::move(other.name);
    this->nodeType = other.nodeType;

    switch (this->nodeType) {
    case (GLTFNodeType::Proxy): {
        new (&this->proxyNode) GLTFProxyNode(std::move(other.proxyNode));
        break;
    }
    case (GLTFNodeType::Mesh): {
        new (&this->meshNode) GLTFMeshNode(std::move(other.meshNode));
        break;
    }
    case (GLTFNodeType::Camera): {
        new (&this->cameraNode) GLTFCameraNode(std::move(other.cameraNode));
        break;
    }
    case (GLTFNodeType::ChildList): {
        new (&this->childListNode) GLTFChildListNode(std::move(other.childListNode));
        break;
    }
    default: {
        std::unreachable();
        break;
    }
    }

    return *this;
}

GLTFTransform GLTFParser::parseTransform(simdjson::ondemand::object node)
{

    auto matrixField = node.find_field_unordered("matrix");
    if (matrixField.has_value()) {
        auto matrixArray = matrixField.get_array();
        ASSERT(matrixArray.count_elements() == (4 * 4));

        float m[16] = { 0 };
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(0).get_double();

        matrixArray->reset();
        return GLTFTransformMatrix {
            .modelMatrix = glm::mat4x4(
                m[0], m[1], m[2], m[3],
                m[4], m[5], m[6], m[7],
                m[8], m[9], m[10], m[11],
                m[12], m[13], m[14], m[15]),
        };
    } else {
        glm::vec3 translation = glm::vec3(0.0f);
        glm::vec4 rotation = glm::vec4(0.0f);
        glm::vec3 scale = glm::vec3(1.0f);

        auto translationField = node.find_field_unordered("translation");
        if (translationField.has_value()) {
            auto translationArray = translationField.get_array();
            ASSERT(translationArray.count_elements() == 3);

            translationArray->reset();
            translation.x = (float)translationArray.at(0).get_double();
            translation.y = (float)translationArray.at(1).get_double();
            translation.z = (float)translationArray.at(2).get_double();
        }

        auto rotationField = node.find_field_unordered("rotation");
        if (rotationField.has_value()) {
            auto rotationArray = rotationField.get_array();
            ASSERT(rotationArray.count_elements() == 4);

            rotationArray->reset();
            rotation[0] = (float)rotationArray.at(0).get_double();
            rotation[1] = (float)rotationArray.at(1).get_double();
            rotation[2] = (float)rotationArray.at(2).get_double();
            rotation[3] = (float)rotationArray.at(3).get_double();
        }

        auto scaleField = node.find_field_unordered("scale");
        if (scaleField.has_value()) {
            auto scaleArray = scaleField.get_array();
            ASSERT(scaleArray.count_elements() == 3);

            scaleArray->reset();
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

std::vector<GLTFNode> GLTFParser::parseNodeList(simdjson::ondemand::array nodeList)
{
    // const size_t nodeCount = nodeList.count_elements().take_value();
    std::vector<GLTFNode> result;
    // result.reserve(nodeCount);

    // NOTE: strings are owned by the parser, here we only take a string view into those.
    // As such for any important strings we will need to make copies. I assume that this
    // will generally speaking apply to any other dynamic data that comes from the parser
    size_t i = 0;
    for (auto nodeEntry : nodeList) {

        auto node = nodeEntry.get_object();
        auto meshField = node->find_field_unordered("mesh");
        auto cameraField = node->find_field_unordered("camera");
        auto childListField = node->find_field_unordered("children");

        std::string_view nodeName = node["name"]->get_string().take_value();
        std::cout << nodeName << std::endl;

        if (meshField.has_value()) {
            auto meshID = meshField->get_uint64().value();
            auto transform = parseTransform(node.value());
            result.emplace_back(i, std::string(nodeName), GLTFMeshNode {
                                                              .transform = transform,
                                                              .meshID = meshID,
                                                          });
        } else if (cameraField.has_value()) {
            auto cameraID = cameraField->get_uint64().value();
            auto transform = parseTransform(node.value());
            result.emplace_back(i, std::string(nodeName), GLTFCameraNode {
                                                              .transform = transform,
                                                              .cameraID = cameraID,
                                                          });
        } else if (childListField.has_value()) {
            auto childListArray = childListField->get_array();

            std::vector<std::uint64_t> childList(childListArray.count_elements());
            for (size_t j = 0; j < childListArray.count_elements(); ++j) {
                childList.push_back(childListArray.at(i).get_uint64());
            }

            result.emplace_back(i, std::string(nodeName), GLTFChildListNode {
                                                              .children = childList,
                                                          });
        } else {
            auto transform = parseTransform(node.value());
            result.emplace_back(i, std::string(nodeName), GLTFProxyNode {
                                                              .transform = transform,
                                                          });
        }

        ++i;
    }

    return result;
}

std::optional<GLTFModel> GLTFParser::parse(std::string_view path)
{
    GLTFModel result;

    simdjson::ondemand::parser gltfParser;
    auto rawJSON = simdjson::padded_string::load(path);

    simdjson::ondemand::document gltf = gltfParser.iterate(rawJSON);

    // const auto defaultScene = gltf["scene"].get_uint64().value();
    //  auto sceneNodes = gltf["scenes"]->get_array().at(defaultScene)["nodes"].get_array();

    auto nodeListField = gltf.find_field_unordered("nodes");
    if (!nodeListField.has_value())
        return {};

    result.sceneNodes = parseNodeList(nodeListField->get_array());

    return result;
}

}
