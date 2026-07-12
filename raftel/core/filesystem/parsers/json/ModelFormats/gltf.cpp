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
    , proxyNode({ })
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

std::vector<GLTFNode> GLTFParser::parseNodeList(simdjson::ondemand::array nodeList)
{
    std::vector<GLTFNode> resultGLTFNodes;

    size_t nodeID = 0;
    for (auto node : nodeList) {
        // by default nodes are proxy nodes until something changes that
        bool nodeHasMatrixTransform = false;
        GLTFTransformComponents nodeTransformComponents = { };
        GLTFTransformMatrix nodeTransformMatrix = { };
        GLTFTransform nodeTransform = { };
        GLTFNodeType nodetype = GLTFNodeType::Proxy;
        std::vector<std::uint64_t> nodeChildList;
        std::uint64_t nodeMeshID = 0;
        std::uint64_t nodeCameraID = 0;
        std::string nodeName = "";
        std::vector<std::string> nodeExtensions;

        auto nodeObject = node.get_object().take_value();
        for (auto field : nodeObject) {
            auto fieldName = field.key().take_value();
            if (fieldName == "name") {
                nodeName = field->value().get_string().value();
            } else if (fieldName == "mesh") {
                nodetype = GLTFNodeType::Mesh;
                nodeMeshID = field->value().get_uint64();
            } else if (fieldName == "camera") {
                nodetype = GLTFNodeType::Camera;
                nodeCameraID = field->value().get_uint64();
            } else if (fieldName == "children") {
                nodetype = GLTFNodeType::ChildList;
                auto childListArray = field->value().get_array();
                nodeChildList.reserve(childListArray.count_elements());
                childListArray->reset();
                for (auto childNodeID : childListArray) {
                    nodeChildList.push_back(childNodeID.get_uint64());
                }
            } else if (fieldName == "matrix") {
                nodeHasMatrixTransform = true;
                auto matrixArray = field->value().get_array();

                float m[16] { 0.0f };
                glm::length_t iter = 0;
                for (auto value : matrixArray) {
                    m[iter] = (float)value->get_double();
                    iter++;
                }
                nodeTransformMatrix = GLTFTransformMatrix {
                    .modelMatrix = glm::mat4x4(
                        m[0], m[1], m[2], m[3],
                        m[4], m[5], m[6], m[7],
                        m[8], m[9], m[10], m[11],
                        m[12], m[13], m[14], m[15]),
                };
            } else if (fieldName == "translation") {
                auto translationArray = field->value().get_array().take_value();
                glm::length_t iter = 0;
                for (auto value : translationArray) {
                    nodeTransformComponents.translation[iter] = (float)value.get_double();
                    iter++;
                }
            } else if (fieldName == "rotation") {
                auto rotationArray = field->value().get_array();
                glm::length_t iter = 0;
                for (auto value : rotationArray) {
                    nodeTransformComponents.rotation[iter] = (float)value.get_double();
                    iter++;
                }
            } else if (fieldName == "scale") {
                auto scaleArray = field->value().get_array();
                glm::length_t iter = 0;
                for (auto value : scaleArray) {
                    nodeTransformComponents.scale[iter] = (float)value.get_double();
                    iter++;
                }
            } else if (fieldName == "extensions") {
                // needs propper per  extension handling i guess
                auto extension = field->value().raw_json_token();
                nodeExtensions.emplace_back(extension);
            }
        }

        if (nodeHasMatrixTransform) {
            nodeTransform = nodeTransformMatrix;
        } else {
            nodeTransform = nodeTransformComponents;
        }

        switch (nodetype) {
        case (GLTFNodeType::Proxy): {
            resultGLTFNodes.emplace_back(nodeID, nodeName, GLTFProxyNode {
                                                               .transform = nodeTransform,
                                                           });
            break;
        }
        case (GLTFNodeType::Camera): {
            resultGLTFNodes.emplace_back(nodeID, nodeName, GLTFCameraNode {
                                                               .transform = nodeTransform,
                                                               .cameraID = nodeCameraID,
                                                           });
            break;
        }
        case (GLTFNodeType::Mesh): {
            resultGLTFNodes.emplace_back(nodeID, nodeName, GLTFMeshNode {
                                                               .transform = nodeTransform,
                                                               .meshID = nodeMeshID,
                                                           });
            break;
        }
        case (GLTFNodeType::ChildList): {
            resultGLTFNodes.emplace_back(nodeID, nodeName, GLTFChildListNode {
                                                               .children = std::move(nodeChildList),
                                                           });
            break;
        }
        default: {
            std::unreachable();
            break;
        }
        }

        ++nodeID;
    }

    return resultGLTFNodes;
}

std::vector<GLTFCamera> GLTFParser::parseCameraList(simdjson::ondemand::array cameraList)
{
    std::vector<GLTFCamera> result;

    size_t cameraID = 0;
    for (auto camera : cameraList) {
        std::string cameraName;
        GLTFCameraType cameraType;

        auto cameraObject = camera.get_object().take_value();
        for (auto field : cameraObject) {
            const auto fieldName = field.key().take_value();
            if (fieldName == "type") {
                const auto typeField = field.value().get_string().take_value();
                if (typeField == "perspective") {
                    cameraType = GLTFCameraType::Perspective;
                } else {
                    cameraType = GLTFCameraType::Orthographic;
                }
            } else if (fieldName == "name") {
                const auto nameField = field.value().get_string().take_value();
                cameraName = nameField;
            } else if (fieldName == "perspective") {
                // parse perspective camera parameters
            } else if (fieldName == "orthographic") {
                // parse orthographic camera parameters
            }
        }

        if (cameraType == GLTFCameraType::Perspective) {
            result.push_back(GLTFCamera {
                .id = cameraID,
                .name = cameraName,
                .perspectiveCamera = { },
                .cameraType = cameraType,
            });
        } else {
            result.push_back(GLTFCamera {
                .id = cameraID,
                .name = cameraName,
                .perspectiveCamera = { },
                .cameraType = cameraType,
            });
        }

        cameraID++;
    }

    return { };
}

std::optional<GLTFModel> GLTFParser::parse(std::string_view path)
{
    GLTFModel result;

    simdjson::ondemand::parser gltfParser;
    auto rawJSON = simdjson::padded_string::load(path);

    simdjson::ondemand::document gltf = gltfParser.iterate(rawJSON);

    // const auto defaultScene = gltf["scene"].get_uint64().value();
    //  auto sceneNodes = gltf["scenes"]->get_array().at(defaultScene)["nodes"].get_array();

    auto nodeListField = gltf["nodes"];
    if (!nodeListField.has_value())
        return { };
    result.sceneNodes = parseNodeList(nodeListField->get_array());

    auto cameraListField = gltf["cameras"];
    if (nodeListField.has_value()) {
        auto cameraList = parseCameraList(cameraListField->get_array());
    }

    return std::move(result);
}

}
