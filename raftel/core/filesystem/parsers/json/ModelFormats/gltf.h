#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <GLM/glm.hpp>
#include <simdjson.h>

namespace raftel::parsers::model {

// NOTE: GLTF model matrices are calculated as M = translation * rotation * scale;
// NOTE: should these classes for transforms just be generic instead of GLTF specific things?
struct GLTFTransformComponents {
    glm::vec4 rotation;
    glm::vec3 translation;
    glm::vec3 scale;
};

struct GLTFTransformMatrix {
    glm::mat4x4 modelMatrix;
};

struct GLTFTransform {
    union {
        GLTFTransformComponents componentTransform;
        GLTFTransformMatrix matrixTransform;
    };
    bool isMatrixTransform = false;

    GLTFTransform() = default;
    GLTFTransform(GLTFTransformComponents components);
    GLTFTransform(GLTFTransformMatrix matrix);

    ~GLTFTransform();
    GLTFTransform(const GLTFTransform& other);
    GLTFTransform(GLTFTransform&& other);
    GLTFTransform& operator=(const GLTFTransform& other);
    GLTFTransform& operator=(GLTFTransform&& other);
};

// NOTE: what types of nodes can exist?
//  - An Empty/"Proxy" node. These are just transform containers are can be used to "random" things such as denoting a marker/target (leaf node)
//  - Mesh nodes. These are nodes with transforms but that also point to an associated mesh (leaf node)
//  - Child Lists. These are nodes that act intermediary nodes in the tree and just have a list of nodes that are under it in the tree
//  - Camera nodes. Nodes that contain a transform + a camera ID/Index (camera nodes then contain specific camera "templates")
//      - so we can easily create cameras that have the same properties but in different places looking at different things
enum class GLTFNodeType {
    Proxy,
    Mesh,
    Camera,
    ChildList,
};

struct GLTFProxyNode {
    GLTFTransform transform;
};

struct GLTFMeshNode {
    GLTFTransform transform;
    std::uint64_t meshID;
};

struct GLTFCameraNode {
    GLTFTransform transform;
    std::uint64_t cameraID;
};

struct GLTFChildListNode {
    std::vector<std::uint64_t> children;
};

struct GLTFNode {
public:
    std::uint64_t id;
    std::string name;
    union {
        GLTFProxyNode proxyNode;
        GLTFMeshNode meshNode;
        GLTFCameraNode cameraNode;
        GLTFChildListNode childListNode;
    };
    GLTFNodeType nodeType;

    GLTFNode(std::uint64_t id, std::string name, GLTFProxyNode proxy);
    GLTFNode(std::uint64_t id, std::string name, GLTFMeshNode mesh);
    GLTFNode(std::uint64_t id, std::string name, GLTFCameraNode camera);
    GLTFNode(std::uint64_t id, std::string name, GLTFChildListNode childList);

    GLTFNode();
    GLTFNode(const GLTFNode& other);
    GLTFNode(GLTFNode&& other);
    ~GLTFNode();

    GLTFNode& operator=(const GLTFNode& other);
    GLTFNode& operator=(GLTFNode&& other);
};

struct GLTFModel {
public:
    std::vector<GLTFNode> sceneNodes;
};

struct GLTFParser {
public:
    static std::optional<GLTFModel> parse(std::string_view path);

private:
    static std::vector<GLTFNode> parseNodeList(simdjson::ondemand::array& nodeList);
    static GLTFTransform parseTransform(simdjson::ondemand::object node);
};
}
