#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <GLM/glm.hpp>

namespace raftel::parsers::model {

// NOTE: GLTF model matrices are calculated as M = translation * rotation * scale;
struct GLTFTransformComponents {
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec3 scale;
};

struct GLTFTransformMatrix {
    glm::mat4x4 modelMatrix;
};

struct GLTFNode {
public:
    std::uint64_t id;
    std::string name;
    std::vector<uint64_t> childrenNodeIDs;
    union {
        GLTFTransformComponents transformComponents;
        GLTFTransformMatrix transformMatrix;
    };
    bool isTransformMatrix;
};

struct GLTFModel {
public:
    std::vector<std::uint64_t> sceneNodeIDs;
};

struct GLTFParser {
    static std::optional<GLTFModel> parse(std::string_view path);
};
}
