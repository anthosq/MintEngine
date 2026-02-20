#pragma once
#include <entt/entt.hpp>
#include "asset/asset.h"
#include "uuid.h"
#include "core/time_step.h"


namespace Mint {
    // 设计ECS时注意内存连续的问题
    // 尽量紧凑

    // 把同类型的组件尽可能集中在一起 
    // Entity只是将component联系在一起
    // Entity实质只是一个UUID(？), 各个组件通过相同的UUID来沟通？
    
    class Scene : public Asset {
        public:
        Scene();
        ~Scene();

        // void OnUpdateEditor(TimeStep ts);
        // void OnUpdateRuntime(TimeStep ts);

        // temp
        // entt::entity CreateEntity(const std::string& name = std::string());
        // entt::entity CreateUUIDEntity(UUID uuid, const std::string& name = std::string());

        // entt::entity CreateChildEntity(entt::entity parent, const std::string& name = std::string(), bool sort = true);

        // void DestroyEntity(entt::entity entity);
        // void DestroyEntity(UUID uuid);

        private:
        UUID m_scene_id;
        entt::registry m_registry;
    };
}