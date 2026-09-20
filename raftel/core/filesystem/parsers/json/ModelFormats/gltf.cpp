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

static GLTFPerspectiveCamera parsePerspectiveCameraParameters(simdjson::ondemand::object perspectiveCameraObject)
{
    double aspectRatio = 16.0 / 9.0;
    double yfov = glm::radians(90.0);
    double znear = 0.001;
    double zfar = 1000.0;
    for (auto field : perspectiveCameraObject) {
        const auto fieldName = field.key().take_value();
        if (fieldName == "aspectRation") {
            aspectRatio = field.value().get_double();
        } else if (fieldName == "yfov") {
            yfov = field.value().get_double();
        } else if (fieldName == "znear") {
            znear = field.value().get_double();
        } else if (fieldName == "zfar") {
            zfar = field.value().get_double();
        }
    }
    return {
        .aspectRatio = aspectRatio,
        .yFOV = yfov,
        .zNear = znear,
        .zFar = zfar,
    };
}

static GLTFOrtograhpicCamera parseOrthographicCameraParameters(simdjson::ondemand::object orthographicCameraObject)
{
    double xMag = 1.0;
    double yMag = 1.0;
    double znear = 0.001;
    double zfar = 1000.0;
    for (auto field : orthographicCameraObject) {
        const auto fieldName = field.key().take_value();
        if (fieldName == "xmag") {
            xMag = field.value().get_double();
        } else if (fieldName == "ymag") {
            yMag = field.value().get_double();
        } else if (fieldName == "znear") {
            znear = field.value().get_double();
        } else if (fieldName == "zfar") {
            zfar = field.value().get_double();
        }
    }
    return {
        .xMag = xMag,
        .yMag = yMag,
        .zNear = znear,
        .zFar = zfar,
    };
}

std::vector<GLTFCamera> GLTFParser::parseCameraList(simdjson::ondemand::array cameraList)
{
    std::vector<GLTFCamera> cameras;

    size_t cameraID = 0;
    for (auto camera : cameraList) {
        std::string cameraName;
        GLTFCameraType cameraType = GLTFCameraType::Orthographic;
        GLTFPerspectiveCamera perspectiveCamera;
        GLTFOrtograhpicCamera orthographicCamera;

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
                perspectiveCamera = parsePerspectiveCameraParameters(field->value().get_object());
            } else if (fieldName == "orthographic") {
                orthographicCamera = parseOrthographicCameraParameters(field->value().get_object());
            }
        }

        if (cameraType == GLTFCameraType::Perspective) {
            cameras.push_back(GLTFCamera {
                .id = cameraID,
                .name = cameraName,
                .perspectiveCamera = perspectiveCamera,
                .cameraType = cameraType,
            });
        } else {
            cameras.push_back(GLTFCamera {
                .id = cameraID,
                .name = cameraName,
                .orthographicsCamera = orthographicCamera,
                .cameraType = cameraType,
            });
        }

        cameraID++;
    }

    return cameras;
}

static GLTFPrimitiveAttributes parsePrimitiveAttributes(simdjson::ondemand::object attributes)
{
    std::uint64_t positionIndex = DEFAULT_INDEX;
    std::uint64_t tangentIndex = DEFAULT_INDEX;
    std::uint64_t normalIndex = DEFAULT_INDEX;
    std::uint64_t textureCoords0Index = DEFAULT_INDEX;
    std::uint64_t textureCoords1Index = DEFAULT_INDEX;
    std::uint64_t colorIndex = DEFAULT_INDEX;
    std::uint64_t jointsIndex = DEFAULT_INDEX;
    std::uint64_t weightsIndex = DEFAULT_INDEX;

    for (auto field : attributes) {
        const auto fieldName = field.key().take_value();
        if (fieldName == "POSITION") {
            positionIndex = field.value().get_uint64();
        } else if (fieldName == "TANGENT") {
            tangentIndex = field.value().get_uint64();
        } else if (fieldName == "NORMAL") {
            normalIndex = field.value().get_uint64();
        } else if (fieldName == "TEXCOORD_0") { // NOTE: apparently there can be an infinite amount of TEXCOORD_<N> entries that we might need to handle, so I need to revamp this. same for color, joints and weights.
            textureCoords0Index = field.value().get_uint64();
        } else if (fieldName == "TEXCOORD_1") {
            textureCoords1Index = field.value().get_uint64();
        } else if (fieldName == "COLOR_0") {
            colorIndex = field.value().get_uint64();
        } else if (fieldName == "JOINTS_0") {
            jointsIndex = field.value().get_uint64();
        } else if (fieldName == "WEIGHTS_0") {
            weightsIndex = field.value().get_uint64();
        }
    }

    return GLTFPrimitiveAttributes {
        .positionIndex = positionIndex,
        .tangentIndex = tangentIndex,
        .normalIndex = normalIndex,
        .textureCoords0Index = textureCoords0Index,
        .textureCoords1Index = textureCoords1Index,
        .colorIndex = colorIndex,
        .jointsIndex = jointsIndex,
        .weightsIndex = weightsIndex,
    };
}

std::vector<GLTFPrimitiveMorphTarget> parsePrimitiveMorphTargets(simdjson::ondemand::array morphTargetList)
{
    std::vector<GLTFPrimitiveMorphTarget> morphTargets;
    for (auto morphTargetEntry : morphTargetList) {
        auto morphTargetObject = morphTargetEntry.get_object().take_value();

        GLTFPrimitiveMorphTarget morphTarget;
        for (auto attribute : morphTargetObject) {
            const auto attributeName = attribute.key().take_value();
            if (attributeName == "POSITION") {
                morphTarget.positionTargetDelta = attribute.value().get_uint64();
            } else if (attributeName == "NORMAL") {
                morphTarget.normalTargetDelta = attribute.value().get_uint64();
            } else if (attributeName == "TANGENT") {
                morphTarget.tangentTargetDelta = attribute.value().get_uint64();
            } else {
                LOG_WARNING("Unsupported morph target attribute found: {}", attributeName.raw());
            }
        }
        morphTargets.push_back(morphTarget);
    }
    return morphTargets;
};

std::vector<GLTFMeshPrimitive> parseMeshPrimitiveArray(simdjson::ondemand::array primitiveList)
{
    std::vector<GLTFMeshPrimitive> primitives;
    for (auto primitive : primitiveList) {
        std::uint64_t materialIndex = DEFAULT_INDEX;
        std::uint64_t indicesAccesssorIndex = DEFAULT_INDEX;
        GLTFPrimitiveType primitiveType = GLTFPrimitiveType::Triangle;
        GLTFPrimitiveAttributes primitiveAttributes;
        std::vector<GLTFPrimitiveMorphTarget> morphTargets;

        auto primitiveObject = primitive.get_object().take_value();
        for (auto field : primitiveObject) {
            const auto fieldName = field.key().take_value();
            if (fieldName == "attributes") {
                primitiveAttributes = parsePrimitiveAttributes(field.value().get_object());
            } else if (fieldName == "indices") {
                indicesAccesssorIndex = field.value().get_uint64();
            } else if (fieldName == "material") {
                materialIndex = field.value().get_uint64();
            } else if (fieldName == "mode") {
                primitiveType = (GLTFPrimitiveType)field.value().get_uint64().value();
            } else if (fieldName == "targets") {
                morphTargets = parsePrimitiveMorphTargets(field.value().get_array());
            } else if (fieldName == "extensions") {
                LOG_WARNING("UNHANDLED EXTENSIONS ARRAY IN MESH PRIMITIVE PARSING");
            } else if (fieldName == "extras") {
                LOG_WARNING("UNHANDLED EXTRAS ARRAY IN MESH PRIMITIVE PARSING");
            }
        }

        primitives.push_back(GLTFMeshPrimitive {
            .materialIndex = materialIndex,
            .indicesAcessorIndex = indicesAccesssorIndex,
            .type = primitiveType,
            .attributes = primitiveAttributes,
            .morphTargets = std::move(morphTargets),
        });
    }
    return primitives;
}

std::vector<GLTFMesh> GLTFParser::parseMeshList(simdjson::ondemand::array meshList)
{
    std::vector<GLTFMesh> meshes;
    for (auto mesh : meshList) {
        std::string meshName;
        std::vector<GLTFMeshPrimitive> primitives;
        std::vector<double> morphWeights;

        auto meshObject = mesh.get_object().take_value();
        for (auto field : meshObject) {
            const auto fieldName = field.key().take_value();
            if (fieldName == "primitives") {
                primitives = parseMeshPrimitiveArray(field.value().get_array());
            } else if (fieldName == "name") {
                const auto nameField = field.value().get_string().take_value();
                meshName = nameField;
            } else if (fieldName == "weights") {
                auto weightsArray = field.value().get_array();
                for (auto weight : weightsArray) {
                    morphWeights.push_back(weight.get_double().take_value());
                }
            } else if (fieldName == "extensions") {
                LOG_WARNING("UNHANDLED EXTENSIONS ARRAY IN MESH LIST PARSING");
            } else if (fieldName == "extras") {
                LOG_WARNING("UNHANDLED EXTRAS ARRAY IN MESH LIST PARSING");
            }
        }

        meshes.emplace_back(GLTFMesh {
            .name = std::move(meshName),
            .primitives = std::move(primitives),
            .morphWeights = std::move(morphWeights),
        });
    }

    return meshes;
}

std::vector<double> parseMinMaxArray(simdjson::ondemand::array limitArray)
{
    std::vector<double> result;
    for (auto limit : limitArray) {
        result.push_back(limit.get_double());
    }
    return result;
}

std::vector<GLTFAccessor> GLTFParser::parseAccessorList(simdjson::ondemand::array accesssorList)
{
    std::vector<GLTFAccessor> accesssors;
    for (auto accessor : accesssorList) {
        std::string accesssorName = "";
        size_t bufferViewIndex = 0;
        size_t bufferSize = 0;
        size_t bufferByteOffset = 0;
        GLTFDataType bufferDataType = GLTFDataType::None;
        GLTFElementType bufferElementType = GLTFElementType::None;
        std::vector<double> elementMinLimits;
        std::vector<double> elementMaxLimits;
        bool normalized = false;
        bool sparse = false;

        auto accessorObject = accessor.get_object().take_value();
        for (auto field : accessorObject) {
            const auto fieldName = field.key().take_value();
            if (fieldName == "bufferView") {
                bufferViewIndex = field.value().get_uint64();
            } else if (fieldName == "byteOffset") {
                bufferByteOffset = field.value().get_uint64();
            } else if (fieldName == "componentType") {
                bufferDataType = (GLTFDataType)field.value().get_uint64().take_value();
            } else if (fieldName == "normalized") {
                normalized = field.value().get_bool();
            } else if (fieldName == "count") {
                bufferSize = field.value().get_uint64();
            } else if (fieldName == "type") {
                const auto typeString = field.value().get_string().take_value();
                if (typeString == "SCALAR") {
                    bufferElementType = GLTFElementType::Scalar;
                } else if (typeString == "VEC2") {
                    bufferElementType = GLTFElementType::Vec2;
                } else if (typeString == "VEC3") {
                    bufferElementType = GLTFElementType::Vec3;
                } else if (typeString == "VEC4") {
                    bufferElementType = GLTFElementType::Vec4;
                } else if (typeString == "MAT2") {
                    bufferElementType = GLTFElementType::Mat2x2;
                } else if (typeString == "MAT3") {
                    bufferElementType = GLTFElementType::Mat3x3;
                } else if (typeString == "MAT4") {
                    bufferElementType = GLTFElementType::Mat4x4;
                } else if (typeString == "string") {
                    bufferElementType = GLTFElementType::String;
                } else {
                    LOG_WARNING("Unrecognized GLTF accessor type: {}", typeString);
                }
            } else if (fieldName == "sparse") {
                sparse = field.value().get_bool();
            } else if (fieldName == "name") {
                accesssorName = field.value().get_string().take_value();
            } else if (fieldName == "min") {
                elementMinLimits = parseMinMaxArray(field.value().get_array());
            } else if (fieldName == "max") {
                elementMaxLimits = parseMinMaxArray(field.value().get_array());
            } else if (fieldName == "extensions") {
                LOG_WARNING("Unhandlded GLTF accessor extension list");
            } else if (fieldName == "extras") {
                LOG_WARNING("Unhandled GLTF accessor extras list");
            } else {
                LOG_WARNING("Unrecognized GLTF accessor field: {}", fieldName.raw());
            }
        }

        accesssors.emplace_back(GLTFAccessor {
            .name = std::move(accesssorName),
            .bufferViewIndex = bufferViewIndex,
            .bufferSize = bufferSize,
            .bufferByteOffset = bufferByteOffset,
            .dataType = bufferDataType,
            .elementType = bufferElementType,
            .elementMinLimits = std::move(elementMinLimits),
            .elementMaxLimits = std::move(elementMaxLimits),
            .normalized = normalized,
            .sparse = sparse,
        });
    }
    return accesssors;
}

std::vector<GLTFBufferView> GLTFParser::parseBufferViewList(simdjson::ondemand::array bufferViewList)
{
    std::vector<GLTFBufferView> result;
    for (auto bufferView : bufferViewList) {
        std::string bufferName = "";
        size_t bufferIndex = DEFAULT_INDEX;
        size_t bufferOffset = 0;
        size_t bufferLength = 0;
        size_t bufferStride = 1;
        GLTFBufferType bufferType = GLTFBufferType::None;

        auto bufferViewObject = bufferView.get_object().take_value();
        for (auto field : bufferViewObject) {
            const auto fieldName = field.key().take_value();
            if (fieldName == "buffer") {
                bufferIndex = field.value().get_uint64();
            } else if (fieldName == "byteOffset") {
                bufferOffset = field.value().get_uint64();
            } else if (fieldName == "byteLength") {
                bufferLength = field.value().get_uint64();
            } else if (fieldName == "byteStride") {
                bufferStride = field.value().get_uint64();
            } else if (fieldName == "target") {
                const auto typeValue = field.value().get_uint64().take_value();
                switch (typeValue) {
                case ((std::uint64_t)GLTFBufferType::ArrayBuffer): {
                    bufferType = GLTFBufferType::ArrayBuffer;
                    break;
                }
                case ((std::uint64_t)GLTFBufferType::ElementArrayBuffer): {
                    bufferType = GLTFBufferType::ElementArrayBuffer;
                    break;
                }
                default: {
                    LOG_WARNING("Found unrecognized buffer view type value: {}", typeValue);
                    break;
                }
                }
            } else if (fieldName == "name") {
                bufferName = field.value().get_string().take_value();
            } else if (fieldName == "extensions") {
                LOG_WARNING("Unhandled extensions array for buffer view");
            } else if (fieldName == "extras") {
                LOG_WARNING("Unhandled extras array for buffer view");
            } else {
                LOG_WARNING("Unrecognized buffer view object field: {}", fieldName.raw());
            }
        }

        result.emplace_back(GLTFBufferView {
            .name = std::move(bufferName),
            .bufferIndex = bufferIndex,
            .bufferOffset = bufferOffset,
            .bufferLength = bufferLength,
            .bufferStride = bufferStride,
            .bufferType = bufferType,
        });
    }
    return result;
}

std::vector<GLTFBuffer> GLTFParser::parseBufferList(simdjson::ondemand::array bufferList)
{
    std::vector<GLTFBuffer> result;
    for (auto buffer : bufferList) {
        std::string uri = "";
        std::string name = "";

        auto bufferObject = buffer.get_object().take_value();
        for (auto field : bufferObject) {
            const auto fieldName = field.key().take_value();
            if (fieldName == "uri") {
                uri = field.value().get_string().take_value();
            } else if (fieldName == "byteLength") {
                // NOTE: ignored due to not being needed. could theorethically be used to preallocate the buffer for the string, however, due to the way simdjson works that would be a lot of work to the point of likely taking longer than what is already reported to us by simdjson.
            } else if (fieldName == "name") {
                name = field.value().get_string().take_value();
            } else if (fieldName == "extensions") {
                LOG_WARNING("Unhandled buffer extension list");
            } else if (fieldName == "extras") {
                LOG_WARNING("Unhandled buffer extras list");
            } else {
                LOG_WARNING("Unrecognized buffer field: {}", fieldName.raw());
            }
        }

        result.emplace_back(GLTFBuffer {
            .bufferURI = std::move(uri),
            .name = std::move(name),
        });
    }
    return result;
}

static GLTFTextureInfo parseTextureInfo(simdjson::ondemand::object textureInfoObject)
{
    size_t index = DEFAULT_INDEX;
    size_t meshTextureCoordIndex = GLTFTextureInfo::DEFAULT_MESH_TEXTURE_INDEX;

    for (auto field : textureInfoObject) {
        const auto fieldName = field.key().take_value();
        if (fieldName == "index") {
            index = field.value();
        } else if (fieldName == "texCoord") {
            meshTextureCoordIndex = field.value();
        } else if (fieldName == "extensions") {
            LOG_WARNING("Unhandled GLTF Texture Info extension list");
        } else if (fieldName == "extras") {
            LOG_WARNING("Unhandled GLTF Texture Info extras list");
        } else {
            LOG_ERROR("Unregonized GLTF Texture Info Field: {}", fieldName.raw());
        }
    }

    return GLTFTextureInfo {
        .index = index,
        .meshTextureCoordIndex = meshTextureCoordIndex,
    };
}

static GLTFPbrMetallicRoughness parsePRBMetallicRougness(simdjson::ondemand::object pbrMetallicRoughnessObject)
{
    std::array<double, 4> baseColor = GLTFPbrMetallicRoughness::DEFAULT_BASE_COLOR;
    std::optional<GLTFTextureInfo> baseColorTextureInfo = std::nullopt;

    for (auto field : pbrMetallicRoughnessObject) {
        const auto fieldName = field.key().take_value();
        if (fieldName == "baseColorFactor") {
            LOG_ERROR("Unhandled GLTF PBR Metallic Roughness Base Color Factor Field");
        } else if (fieldName == "baseColorTexture") {
            baseColorTextureInfo = parseTextureInfo(field.value());
        } else if (fieldName == "metallicFactor") {
            LOG_ERROR("Unhandled GLTF PBR Metallic Roughness Metallic Factor Field");
        } else if (fieldName == "roughnessFactor") {
            LOG_ERROR("Unhandled GLTF PBR Metallic Roughness Roughness Factor Field");
        } else if (fieldName == "metallicRoughnessTexture") {
            LOG_ERROR("Unhandled GLTF PBR Metallic Roughness Metallic Roughness Texture Field");
        } else if (fieldName == "extensions") {
            LOG_ERROR("Unhandled GLTF PBR Metallic Roughness Extensions Field");
        } else if (fieldName == "extras") {
            LOG_ERROR("Unhandled GLTF PBR Metallic Roughness Extras Field");
        } else {
            LOG_ERROR("Unrecognized GLTF PBR Metallic Roughness Field: {}", fieldName.raw());
        }
    }

    return {
        .baseColor = baseColor,
    };
}

std::vector<GLTFMaterial> GLTFParser::parseMaterialList(simdjson::ondemand::array materialList)
{
    std::vector<GLTFMaterial> result;
    for (auto material : materialList) {
        std::string materialName = "";
        GLTFPbrMetallicRoughness metallicRoughness = { };

        auto materialObject = material.get_object().take_value();
        for (auto field : materialObject) {
            const auto fieldName = field.key().take_value();
            if (fieldName == "name") {
                materialName = field.value().get_string().take_value();
            } else if (fieldName == "pbrMetallicRoughness") {
                metallicRoughness = parsePRBMetallicRougness(field.value());
            } else if (fieldName == "normalTexture") {
                LOG_WARNING("Unhandled GLTF Material Normal Texture Field");
            } else if (fieldName == "occlusionTexture") {
                LOG_WARNING("Unhandled GLTF Material Occlusion Texture Field");
            } else if (fieldName == "emissiveTexture") {
                LOG_WARNING("Unhandled GLTF Material Emissive Texture Field");
            } else if (fieldName == "emissiveFactor") {
                LOG_WARNING("Unhandled GLTF Material Emissive Factors Field");
            } else if (fieldName == "alphaMode") {
                LOG_WARNING("Unhandled GLTF Material Alpha Mode Field");
            } else if (fieldName == "alphaCutoff") {
                LOG_WARNING("Unhandled GLTF Material Alpha Cutoff Field");
            } else if (fieldName == "doubleSided") {
                LOG_WARNING("Unhandled GLTF Material Double Sided Field");
            } else if (fieldName == "extensions") {
                LOG_WARNING("Unhandled GLTF Material Extensions Field");
            } else if (fieldName == "extras") {
                LOG_WARNING("Unhandled GLTF Material Extras Field");
            } else {
                LOG_WARNING("Unrecognized GLTF Material Field: {}", fieldName.raw());
            }
        }

        result.emplace_back(GLTFMaterial {
            .name = std::move(materialName),
        });
    }
    return result;
}

/*
 *  List of Root-Level GLTF Fields as per specification:
 *   - extensionsUsed           (string array)
 *   - extensionsRequired       (string array)
 *   - accessors                (accessor array)
 *   - animations               (animation array)
 *   - asset                    (asset array)
 *   - buffers                  (buffer array)
 *   - bufferViews              (bufferView array)
 *   - cameras                  (camera array)
 *   - images                   (image array)
 *   - materials                (material array)
 *   - meshes                   (mesh array)
 *   - nodes                    (node array)
 *   - samplers                 (sampler array)
 *   - scene                    (default scene index)
 *   - scenes                   (scene array)
 *   - skins                    (skin array)
 *   - textures                 (texture array)
 */

std::optional<GLTFModel> GLTFParser::parse(std::string_view path)
{
    GLTFModel result;

    simdjson::ondemand::parser gltfParser;
    auto rawJSON = simdjson::padded_string::load(path);

    simdjson::ondemand::document gltf = gltfParser.iterate(rawJSON);

    // const auto defaultScene = gltf["scene"].get_uint64().value();
    //  auto sceneNodes = gltf["scenes"]->get_array().at(defaultScene)["nodes"].get_array();

    for (auto field : gltf.get_object()) {
        const auto fieldName = field.key().take_value();
        auto value = field.value();
        if (fieldName == "nodes") {
            result.sceneNodes = parseNodeList(value);
        } else if (fieldName == "cameras") {
            result.cameras = parseCameraList(value);
        } else if (fieldName == "meshes") {
            result.meshes = parseMeshList(value);
        } else if (fieldName == "accessors") {
            result.accessors = parseAccessorList(value);
        } else if (fieldName == "bufferViews") {
            result.bufferViews = parseBufferViewList(value);
        } else if (fieldName == "buffers") {
            result.buffers = parseBufferList(value);
        } else if (fieldName == "animations") {
            LOG_WARNING("Unhandled GLTF Animation Array");
        } else if (fieldName == "asset") {
            LOG_WARNING("Unhandled GLTF Asset Array");
        } else if (fieldName == "images") {
            LOG_WARNING("Unhandled GLTF Image Array");
        } else if (fieldName == "materials") {
            result.materials = parseMaterialList(value);
        } else if (fieldName == "skins") {
            LOG_WARNING("Unhandled GLTF Skin Array");
        } else if (fieldName == "textures") {
            LOG_WARNING("Unhandled GLTF Texture Array");
        } else if (fieldName == "scene") {
            LOG_WARNING("Unhandled GLTF Default Scene Index");
        } else if (fieldName == "scenes") {
            LOG_WARNING("Unhandled GLTF Scene Array");
        } else if (fieldName == "samplers") {
            LOG_WARNING("Unhandled GLTF Sampler Array");
        } else if (fieldName == "extensionsUsed") {
            LOG_WARNING("Unhandled GLTF Used Extension Array");
        } else if (fieldName == "extensionsRequired") {
            LOG_WARNING("Unhandled GLTF Required Extension Array");
        } else if (fieldName == "extensions") {
            LOG_WARNING("Unhandled GLTF Extensions Array");
        } else if (fieldName == "extras") {
            LOG_WARNING("Unhandled GLTF Extras Array");
        } else {
            LOG_WARNING("Unrecognized GLTF Field: {}", fieldName.raw());
        }
    }

    return std::move(result);
}
}
