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

GLTFTransform GLTFParser::parseTransform(simdjson::ondemand::object node)
{

    auto matrixField = node.find_field_unordered("matrix");
    if (matrixField.has_value()) {
        auto matrixArray = matrixField.get_array();
        // ASSERT(matrixArray.count_elements() == (4 * 4));

        float m[16] = { 0 };
        m[0] = (float)matrixArray.at(0).get_double();
        m[0] = (float)matrixArray.at(1).get_double();
        m[0] = (float)matrixArray.at(2).get_double();
        m[0] = (float)matrixArray.at(3).get_double();
        m[0] = (float)matrixArray.at(4).get_double();
        m[0] = (float)matrixArray.at(5).get_double();
        m[0] = (float)matrixArray.at(6).get_double();
        m[0] = (float)matrixArray.at(7).get_double();
        m[0] = (float)matrixArray.at(8).get_double();
        m[0] = (float)matrixArray.at(9).get_double();
        m[0] = (float)matrixArray.at(10).get_double();
        m[0] = (float)matrixArray.at(11).get_double();
        m[0] = (float)matrixArray.at(12).get_double();
        m[0] = (float)matrixArray.at(13).get_double();
        m[0] = (float)matrixArray.at(14).get_double();
        m[0] = (float)matrixArray.at(15).get_double();

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
            auto translationArray = translationField.get_array().take_value();
            // ASSERT(translationArray.count_elements() == 3);

            int i = 0;
            for (auto value : translationArray) {
                translation[i] = (float)value->get_double();
                i++;
            }
        }

        auto rotationField = node.find_field_unordered("rotation");
        if (rotationField.has_value()) {
            auto rotationArray = rotationField.get_array().take_value();
            // ASSERT(rotationArray.count_elements() == 4);

            int i = 0;
            for (auto value : rotationArray) {
                rotation[i] = (float)value.get_double();
                i++;
            }
        }

        auto scaleField = node.find_field_unordered("scale");
        if (scaleField.has_value()) {
            auto scaleArray = scaleField.get_array().take_value();
            // ASSERT(scaleArray.count_elements() == 3);

            int i = 0;
            for (auto value : scaleArray) {
                scale[i] = (float)value.get_double();
                i++;
            }
        }

        return GLTFTransformComponents {
            .rotation = rotation,
            .translation = translation,
            .scale = scale,
        };
    }
}

#if 0
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
        std::string_view nodeName = node["name"]->get_string().take_value();
        std::cout << nodeName << std::endl;

        auto meshField = node->find_field_unordered("mesh");
        if (meshField.has_value()) {
            auto meshID = meshField->get_uint64().value();
            auto transform = parseTransform(*node);
            result.emplace_back(i, std::string(nodeName), GLTFMeshNode {
                                                              .transform = transform,
                                                              .meshID = meshID,
                                                          });
            ++i;
            continue;
        }

        auto cameraField = node->find_field_unordered("camera");
        if (cameraField.has_value()) {
            auto cameraID = cameraField->get_uint64().value();
            auto transform = parseTransform(*node);
            result.emplace_back(i, std::string(nodeName), GLTFCameraNode {
                                                              .transform = transform,
                                                              .cameraID = cameraID,
                                                          });
            ++i;
            continue;
        }

        auto childListField = node->find_field_unordered("children");
        if (childListField.has_value()) {
            auto childListArray = childListField->get_array();
            const auto childCount = childListArray.count_elements().value();
            childListArray->reset();

            std::vector<std::uint64_t> childList(childCount);
            for (auto childID : childListArray) {
                childList.push_back(childID);
            }

            result.emplace_back(i, std::string(nodeName), GLTFChildListNode {
                                                              .children = childList,
                                                          });
            ++i;
            continue;
        }

        auto transform = parseTransform(*node);
        result.emplace_back(i, std::string(nodeName), GLTFProxyNode {
                                                          .transform = transform,
                                                      });
        ++i;
    }

    return result;
}
#else
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

    return { };
}
#endif

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
        return { };

    result.sceneNodes = parseNodeList(nodeListField->get_array());

    return result;
}

}
