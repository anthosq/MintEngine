#include "scene.h"
#include "glm/glm.hpp"

namespace Mint {

    Scene::Scene() {

        // entt::entity entity = m_registry.create();
        // m_registry.emplace<TransformComponent>(entity, glm::vec3...);


        // on_construct...
        // m_registry.on_construct<TransformComponent>().connect<&Scene::OnTransformComponentConstruct>(this);

        // auto view = m_registry.view<TransformComponent>();
        // for (auto entity : view) {
        //     TransformComponent &transform = view.get<TransformComponent>(entity);
        // }

        // auto group = m_registry.group<TransformComponent>(entt::get<MeshComponent>);
        // for (auto entity : group) {
        //     auto& [transform, mesh] = group.get<TransformComponent, MeshComponent>(entity);
        //     RenderSystem::Submit(mesh, transform);
        // }

    }

    Scene::~Scene() {

    }

    // entt::entity CreateEntity(const std::string &name = std::string()) {
    //     entt::entity entity = m_registry.create();
    //     m_registry.emplace<NameComponent>(entity, name);
    //     return entity;
    // }

}