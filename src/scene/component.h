#pragma once

#include <entt/entt.hpp>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "uuid.h"
#include "scene_camera.h"
#include "render/texture.h"


namespace Mint {

    struct IDComponent {
        UUID ID = 0;
    };

    struct TagComponent {
        std::string Tag;
        TagComponent() = default;
        TagComponent(const TagComponent& other) = default;
        TagComponent(const std::string& tag) : Tag(tag) {}
    
        operator std::string& () { return Tag; }
        operator const std::string& () const { return Tag; }
    };


    struct RelationshipComponent
    {
        UUID Parent = 0;
        std::vector<UUID> Children;

        RelationshipComponent() = default;
        RelationshipComponent(const RelationshipComponent& other) = default;
        RelationshipComponent(UUID parent) : Parent(parent) {}
    };

    struct PrefabComponent
    {
        UUID PrefabID = 0;
        UUID EntityID = 0;
    };

    struct TransformComponent
    {
        glm::vec3 Translation = glm::vec3(0.0f);
        glm::vec3 Scale = glm::vec3(1.0f);

    private:
        glm::quat Rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        glm::vec3 RotationEuler = glm::vec3(0.0f);

    public:
        TransformComponent() = default;
        TransformComponent(const TransformComponent& other) = default;
        TransformComponent(const glm::vec3& translation) : Translation(translation) {}

        glm::mat4 GetTransform() const {
            return glm::translate(glm::mat4(1.0f), Translation)
                * glm::toMat4(Rotation)
                * glm::scale(glm::mat4(1.0f), Scale);
        }

        void SetTransform(const glm::mat4& transform) {
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(transform, Scale, Rotation, Translation, skew, perspective);
            RotationEuler = glm::eulerAngles(Rotation);
            return;
        }

        glm::vec3 GetRotationEuler() const {
            return RotationEuler;
        }

        void SetRotationEuler(const glm::vec3& euler) {
            RotationEuler = euler;
            Rotation = glm::quat(RotationEuler);
            return;
        }

        glm::quat GetRotation() const {
            return Rotation;
        }

        void SetRotation(const glm::quat& quat) {
            // 这里需要考虑欧拉角的跳变问题
            auto warpToPi = [](glm::vec3 v) {
                return glm::mod(v + glm::pi<float>(), 2.0f * glm::pi<float>()) - glm::pi<float>();
            };

            auto originalEuler = RotationEuler;
            Rotation = quat;
            RotationEuler = glm::eulerAngles(Rotation);

            // alternate
            glm::vec3 alternate1 = { RotationEuler.x - glm::pi<float>(), glm::pi<float>() - RotationEuler.y, RotationEuler.z - glm::pi<float>() };            
            glm::vec3 alternate2 = { RotationEuler.x + glm::pi<float>(), glm::pi<float>() - RotationEuler.y, RotationEuler.z - glm::pi<float>() };            
            glm::vec3 alternate3 = { RotationEuler.x + glm::pi<float>(), glm::pi<float>() - RotationEuler.y, RotationEuler.z + glm::pi<float>() };            
            glm::vec3 alternate4 = { RotationEuler.x - glm::pi<float>(), glm::pi<float>() - RotationEuler.y, RotationEuler.z + glm::pi<float>() };            

            float distance0 = glm::length2(warpToPi(RotationEuler - originalEuler));
            float distance1 = glm::length2(warpToPi(alternate1 - originalEuler));
            float distance2 = glm::length2(warpToPi(alternate2 - originalEuler));
            float distance3 = glm::length2(warpToPi(alternate3 - originalEuler));
            float distance4 = glm::length2(warpToPi(alternate4 - originalEuler));

            float minimum = distance0;
            if (distance1 < minimum) {
                minimum = distance1;
                RotationEuler = alternate1;
            }
            if (distance2 < minimum) {
                minimum = distance2;
                RotationEuler = alternate2;
            }
            if (distance3 < minimum) {
                minimum = distance3;
                RotationEuler = alternate3;
            }
            if (distance4 < minimum) {
                minimum = distance4;
                RotationEuler = alternate4;
            }

            RotationEuler = warpToPi(RotationEuler);
            return;
        }

    };
    
    struct CameraComponent {
        enum class Type {
            Perspective,
            Orthographic
        };
        Type ProjectionType;

        SceneCamera Camera;
        bool Primary = true;

        CameraComponent() = default;
        CameraComponent(const CameraComponent& other) = default;

        operator SceneCamera& () { return Camera; }
        operator const SceneCamera& () const { return Camera; }
    };


    struct SpriteRendererComponent {
        glm::vec4 Color = glm::vec4(1.0f);
        Ref<Texture2D> Texture;
        float TilingFactor = 1.0f;

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent& other) = default;
        SpriteRendererComponent(const glm::vec4& color) : Color(color) {}
    };

    struct CubeRendererComponent {
        Ref<Texture2D> Texture;


    }


}