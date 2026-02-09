#pragma once
#include <glm/glm.hpp>

namespace Mint {
    struct AABB {
        glm::vec3 Min;
        glm::vec3 Max;

        AABB() : Min(glm::vec3(0.0f)), Max(glm::vec3(0.0f)) {
        }

        glm::vec3 Size() { return Max - Min; }
        glm::vec3 Center() { return (Min + Max) * 0.5f; }

    };
}