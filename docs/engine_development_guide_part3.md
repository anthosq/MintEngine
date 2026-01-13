# MintEngine 引擎开发指南 (Part 3)

> 本部分讨论动画系统、编辑器开发、项目架构等高级主题。

---

## 目录

17. [骨骼动画系统](#17-骨骼动画系统)
18. [编辑器开发](#18-编辑器开发)
19. [序列化系统](#19-序列化系统)
20. [项目架构建议](#20-项目架构建议)
21. [引擎开发常见陷阱](#21-引擎开发常见陷阱)
22. [学习路线图](#22-学习路线图)

---

## 17. 骨骼动画系统

### 17.1 骨骼动画原理

```
┌─────────────────────────────────────────────────────────────┐
│                    Skeletal Animation                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Skeleton (骨架)         Animation Clip (动画片段)           │
│  ┌─────────────┐         ┌─────────────────┐               │
│  │  Bone 0     │         │ KeyFrame 0 (t=0)│               │
│  │    └─ Bone 1│         │ KeyFrame 1 (t=1)│               │
│  │        └─ Bone 2      │ KeyFrame 2 (t=2)│               │
│  │    └─ Bone 3│         │     ...         │               │
│  └─────────────┘         └─────────────────┘               │
│         │                        │                          │
│         ▼                        ▼                          │
│  ┌─────────────────────────────────────┐                   │
│  │         Animator (动画器)           │                   │
│  │   - 采样关键帧，插值计算             │                   │
│  │   - 计算每根骨骼的最终变换           │                   │
│  │   - 生成蒙皮矩阵数组                 │                   │
│  └─────────────────────────────────────┘                   │
│                     │                                       │
│                     ▼                                       │
│  ┌─────────────────────────────────────┐                   │
│  │           GPU Skinning              │                   │
│  │   vertex' = Σ(weight_i * bone_i * vertex)              │
│  └─────────────────────────────────────┘                   │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 17.2 骨骼和动画数据结构

```cpp
// ============ src/animation/skeleton.h ============

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>

namespace Mint {

    // 骨骼节点
    struct Bone {
        std::string name;
        int parentIndex = -1;  // -1 表示根骨骼

        // 绑定姿势 (Bind Pose) - 模型空间
        glm::mat4 localBindTransform;   // 相对于父骨骼的变换
        glm::mat4 inverseBindPose;      // 从模型空间到骨骼空间

        // 当前姿势
        glm::vec3 localPosition;
        glm::quat localRotation;
        glm::vec3 localScale = glm::vec3(1.0f);
    };

    // 骨架
    class Skeleton : public RefCounter {
    public:
        Skeleton() = default;

        // 添加骨骼
        int AddBone(const std::string& name, int parentIndex,
                    const glm::mat4& localBindTransform);

        // 查找骨骼
        int FindBoneIndex(const std::string& name) const;
        Bone* FindBone(const std::string& name);
        const Bone* FindBone(const std::string& name) const;

        // 计算全局变换
        void CalculateGlobalTransforms(std::vector<glm::mat4>& globalTransforms) const;

        // 计算蒙皮矩阵
        void CalculateSkinningMatrices(const std::vector<glm::mat4>& globalTransforms,
                                       std::vector<glm::mat4>& skinningMatrices) const;

        // Getter
        const std::vector<Bone>& GetBones() const { return m_bones; }
        std::vector<Bone>& GetBones() { return m_bones; }
        size_t GetBoneCount() const { return m_bones.size(); }

    private:
        std::vector<Bone> m_bones;
        std::unordered_map<std::string, int> m_boneNameToIndex;
    };

} // namespace Mint
```

```cpp
// ============ src/animation/animation_clip.h ============

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <string>

namespace Mint {

    // 关键帧
    template<typename T>
    struct KeyFrame {
        float time;
        T value;
    };

    using PositionKeyFrame = KeyFrame<glm::vec3>;
    using RotationKeyFrame = KeyFrame<glm::quat>;
    using ScaleKeyFrame = KeyFrame<glm::vec3>;

    // 单个骨骼的动画轨道
    struct BoneAnimationTrack {
        std::string boneName;
        int boneIndex = -1;

        std::vector<PositionKeyFrame> positionKeys;
        std::vector<RotationKeyFrame> rotationKeys;
        std::vector<ScaleKeyFrame> scaleKeys;

        // 在指定时间采样
        glm::vec3 SamplePosition(float time) const;
        glm::quat SampleRotation(float time) const;
        glm::vec3 SampleScale(float time) const;
        glm::mat4 SampleTransform(float time) const;
    };

    // 动画片段
    class AnimationClip : public RefCounter, public Asset {
    public:
        AnimationClip() = default;
        AnimationClip(const std::string& name, float duration);

        // 添加轨道
        void AddTrack(const BoneAnimationTrack& track);

        // 采样整个动画
        void Sample(float time, Skeleton& skeleton) const;

        // Getter
        const std::string& GetName() const { return m_name; }
        float GetDuration() const { return m_duration; }
        float GetTicksPerSecond() const { return m_ticksPerSecond; }
        const std::vector<BoneAnimationTrack>& GetTracks() const { return m_tracks; }

        // Asset 接口
        MINT_ASSET_TYPE(Animation);

    private:
        std::string m_name;
        float m_duration = 0.0f;
        float m_ticksPerSecond = 25.0f;
        std::vector<BoneAnimationTrack> m_tracks;
    };

} // namespace Mint
```

### 17.3 动画器实现

```cpp
// ============ src/animation/animator.h ============

#pragma once
#include "animation/skeleton.h"
#include "animation/animation_clip.h"

namespace Mint {

    // 动画混合模式
    enum class BlendMode {
        Override,   // 完全覆盖
        Additive,   // 叠加
        Blend       // 混合
    };

    // 动画状态
    struct AnimationState {
        Ref<AnimationClip> clip;
        float time = 0.0f;
        float speed = 1.0f;
        float weight = 1.0f;
        bool loop = true;
        BlendMode blendMode = BlendMode::Override;

        bool isPlaying = false;
    };

    // 动画器
    class Animator : public RefCounter {
    public:
        Animator(const Ref<Skeleton>& skeleton);

        // 播放控制
        void Play(const Ref<AnimationClip>& clip, float transitionTime = 0.2f);
        void PlayImmediate(const Ref<AnimationClip>& clip);
        void Stop();
        void Pause();
        void Resume();

        // 混合播放
        void CrossFade(const Ref<AnimationClip>& clip, float duration);
        void SetLayerWeight(int layer, float weight);

        // 更新
        void Update(float deltaTime);

        // 获取蒙皮矩阵 (用于 GPU Skinning)
        const std::vector<glm::mat4>& GetSkinningMatrices() const {
            return m_skinningMatrices;
        }

        // 上传到 GPU
        void UploadToGPU();

        // Getter
        Ref<Skeleton> GetSkeleton() const { return m_skeleton; }
        float GetCurrentTime() const { return m_currentState.time; }
        bool IsPlaying() const { return m_currentState.isPlaying; }

    private:
        void UpdateBoneTransforms();
        void BlendStates(float blendFactor);

    private:
        Ref<Skeleton> m_skeleton;
        Ref<UniformBuffer> m_boneUBO;

        // 动画状态
        AnimationState m_currentState;
        AnimationState m_previousState;  // 用于过渡
        float m_transitionProgress = 1.0f;
        float m_transitionDuration = 0.0f;

        // 计算结果
        std::vector<glm::mat4> m_globalTransforms;
        std::vector<glm::mat4> m_skinningMatrices;
    };

} // namespace Mint
```

```cpp
// ============ src/animation/animator.cpp ============

#include "animation/animator.h"

namespace Mint {

    static const uint32_t MAX_BONES = 128;

    Animator::Animator(const Ref<Skeleton>& skeleton)
        : m_skeleton(skeleton) {

        size_t boneCount = skeleton->GetBoneCount();
        m_globalTransforms.resize(boneCount);
        m_skinningMatrices.resize(boneCount);

        // 创建骨骼 UBO
        m_boneUBO = UniformBuffer::Create(
            MAX_BONES * sizeof(glm::mat4),
            UniformBindingPoints::Skinning
        );

        // 初始化为单位矩阵
        for (size_t i = 0; i < boneCount; i++) {
            m_skinningMatrices[i] = glm::mat4(1.0f);
        }
    }

    void Animator::Play(const Ref<AnimationClip>& clip, float transitionTime) {
        if (m_currentState.isPlaying && transitionTime > 0.0f) {
            // 保存当前状态用于过渡
            m_previousState = m_currentState;
            m_transitionDuration = transitionTime;
            m_transitionProgress = 0.0f;
        }

        m_currentState.clip = clip;
        m_currentState.time = 0.0f;
        m_currentState.isPlaying = true;
    }

    void Animator::Update(float deltaTime) {
        if (!m_currentState.isPlaying || !m_currentState.clip) return;

        // 更新时间
        m_currentState.time += deltaTime * m_currentState.speed;

        float duration = m_currentState.clip->GetDuration();
        if (m_currentState.loop) {
            m_currentState.time = fmod(m_currentState.time, duration);
        } else if (m_currentState.time >= duration) {
            m_currentState.time = duration;
            m_currentState.isPlaying = false;
        }

        // 更新过渡
        if (m_transitionProgress < 1.0f) {
            m_transitionProgress += deltaTime / m_transitionDuration;
            m_transitionProgress = std::min(m_transitionProgress, 1.0f);
        }

        UpdateBoneTransforms();
    }

    void Animator::UpdateBoneTransforms() {
        // 采样当前动画
        m_currentState.clip->Sample(m_currentState.time, *m_skeleton);

        // 如果在过渡中，混合两个动画
        if (m_transitionProgress < 1.0f && m_previousState.clip) {
            // 保存当前采样结果
            auto currentBones = m_skeleton->GetBones();

            // 采样前一个动画
            m_previousState.clip->Sample(m_previousState.time, *m_skeleton);

            // 混合
            float t = m_transitionProgress;
            auto& bones = m_skeleton->GetBones();
            for (size_t i = 0; i < bones.size(); i++) {
                bones[i].localPosition = glm::mix(bones[i].localPosition,
                                                   currentBones[i].localPosition, t);
                bones[i].localRotation = glm::slerp(bones[i].localRotation,
                                                    currentBones[i].localRotation, t);
                bones[i].localScale = glm::mix(bones[i].localScale,
                                                currentBones[i].localScale, t);
            }
        }

        // 计算全局变换
        m_skeleton->CalculateGlobalTransforms(m_globalTransforms);

        // 计算蒙皮矩阵
        m_skeleton->CalculateSkinningMatrices(m_globalTransforms, m_skinningMatrices);
    }

    void Animator::UploadToGPU() {
        m_boneUBO->SetData(m_skinningMatrices.data(),
                          m_skinningMatrices.size() * sizeof(glm::mat4));
    }

} // namespace Mint
```

### 17.4 GPU Skinning 着色器

```glsl
// ============ shaders/skinned_mesh.glsl ============

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in ivec4 a_BoneIDs;     // 最多4个骨骼影响
layout(location = 4) in vec4 a_BoneWeights;  // 对应权重

// Camera UBO
layout(std140, binding = 0) uniform CameraData {
    mat4 u_ViewProjection;
    mat4 u_View;
    mat4 u_Projection;
    vec3 u_CameraPosition;
};

// Transform UBO
layout(std140, binding = 1) uniform TransformData {
    mat4 u_Model;
    mat4 u_NormalMatrix;
};

// Bone Matrices UBO
const int MAX_BONES = 128;
layout(std140, binding = 5) uniform BoneData {
    mat4 u_BoneMatrices[MAX_BONES];
};

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} vs_out;

void main() {
    // 计算蒙皮变换
    mat4 skinMatrix = mat4(0.0);

    for (int i = 0; i < 4; i++) {
        if (a_BoneIDs[i] >= 0 && a_BoneIDs[i] < MAX_BONES) {
            skinMatrix += u_BoneMatrices[a_BoneIDs[i]] * a_BoneWeights[i];
        }
    }

    // 如果没有骨骼影响，使用单位矩阵
    if (a_BoneWeights.x + a_BoneWeights.y + a_BoneWeights.z + a_BoneWeights.w < 0.01) {
        skinMatrix = mat4(1.0);
    }

    // 应用蒙皮变换
    vec4 skinnedPosition = skinMatrix * vec4(a_Position, 1.0);
    vec4 worldPosition = u_Model * skinnedPosition;

    // 法线变换
    mat3 skinNormalMatrix = mat3(transpose(inverse(skinMatrix)));
    vec3 skinnedNormal = normalize(skinNormalMatrix * a_Normal);
    vec3 worldNormal = normalize(mat3(u_NormalMatrix) * skinnedNormal);

    vs_out.FragPos = worldPosition.xyz;
    vs_out.Normal = worldNormal;
    vs_out.TexCoord = a_TexCoord;

    gl_Position = u_ViewProjection * worldPosition;
}

#type fragment
#version 450 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;

out vec4 FragColor;

// 材质纹理
uniform sampler2D u_AlbedoMap;

// 包含光照计算
#include "common/lighting.glsl"

void main() {
    vec3 albedo = texture(u_AlbedoMap, fs_in.TexCoord).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(u_CameraPosition - fs_in.FragPos);

    SurfaceData surface;
    surface.position = fs_in.FragPos;
    surface.normal = normal;
    surface.viewDir = viewDir;
    surface.albedo = albedo;
    surface.shininess = 32.0;

    vec3 result = CalculateLighting(surface);

    FragColor = vec4(result, 1.0);
}
```

---

## 18. 编辑器开发

### 18.1 编辑器架构

```
┌─────────────────────────────────────────────────────────────┐
│                      Editor Application                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │                    EditorLayer                       │   │
│  │  ┌─────────┬─────────┬─────────┬─────────────────┐ │   │
│  │  │ Scene   │Inspector│Viewport │ Content Browser │ │   │
│  │  │Hierarchy│  Panel  │  Panel  │     Panel       │ │   │
│  │  └─────────┴─────────┴─────────┴─────────────────┘ │   │
│  └─────────────────────────────────────────────────────┘   │
│                           │                                 │
│                           ▼                                 │
│  ┌─────────────────────────────────────────────────────┐   │
│  │                  Editor Services                     │   │
│  │  - Selection Manager                                 │   │
│  │  - Undo/Redo System                                  │   │
│  │  - Asset Importer                                    │   │
│  │  - Project Manager                                   │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 18.2 EditorLayer 实现

```cpp
// ============ src/editor/editor_layer.h ============

#pragma once
#include "layer.h"
#include "scene/scene.h"
#include "scene/entity.h"
#include "render/framebuffer.h"
#include "editor/editor_camera.h"

namespace Mint {

    class EditorLayer : public Layer {
    public:
        EditorLayer();
        virtual ~EditorLayer() = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(TimeStep ts) override;
        void OnImGuiRender() override;
        void OnEvent(Event& e) override;

    private:
        // UI 面板
        void DrawMenuBar();
        void DrawSceneHierarchyPanel();
        void DrawInspectorPanel();
        void DrawViewportPanel();
        void DrawContentBrowserPanel();
        void DrawToolbar();

        // 场景操作
        void NewScene();
        void OpenScene();
        void SaveScene();
        void SaveSceneAs();

        // 事件处理
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

        // Gizmo
        void DrawGizmos();

    private:
        Ref<Scene> m_activeScene;
        Ref<Scene> m_editorScene;

        Ref<Framebuffer> m_viewportFramebuffer;
        EditorCamera m_editorCamera;

        Entity m_selectedEntity;
        Entity m_hoveredEntity;

        // 视口状态
        glm::vec2 m_viewportSize = { 0, 0 };
        glm::vec2 m_viewportBounds[2];
        bool m_viewportFocused = false;
        bool m_viewportHovered = false;

        // Gizmo
        int m_gizmoType = -1;  // -1: none, 0: translate, 1: rotate, 2: scale

        // 文件路径
        std::filesystem::path m_currentScenePath;
    };

} // namespace Mint
```

### 18.3 Scene Hierarchy Panel

```cpp
// ============ 场景层级面板 ============

void EditorLayer::DrawSceneHierarchyPanel() {
    ImGui::Begin("Scene Hierarchy");

    // 右键菜单 - 创建实体
    if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_NoOpenOverItems |
                                          ImGuiPopupFlags_MouseButtonRight)) {
        if (ImGui::MenuItem("Create Empty Entity")) {
            m_activeScene->CreateEntity("Empty Entity");
        }
        if (ImGui::BeginMenu("3D Object")) {
            if (ImGui::MenuItem("Cube")) {
                auto entity = m_activeScene->CreateEntity("Cube");
                entity.AddComponent<MeshRendererComponent>(
                    MeshFactory::CreateCube()
                );
            }
            if (ImGui::MenuItem("Sphere")) {
                auto entity = m_activeScene->CreateEntity("Sphere");
                entity.AddComponent<MeshRendererComponent>(
                    MeshFactory::CreateSphere()
                );
            }
            if (ImGui::MenuItem("Plane")) {
                auto entity = m_activeScene->CreateEntity("Plane");
                entity.AddComponent<MeshRendererComponent>(
                    MeshFactory::CreatePlane()
                );
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Light")) {
            if (ImGui::MenuItem("Directional Light")) {
                auto entity = m_activeScene->CreateEntity("Directional Light");
                entity.AddComponent<DirectionalLightComponent>();
            }
            if (ImGui::MenuItem("Point Light")) {
                auto entity = m_activeScene->CreateEntity("Point Light");
                entity.AddComponent<PointLightComponent>();
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Camera")) {
            auto entity = m_activeScene->CreateEntity("Camera");
            entity.AddComponent<CameraComponent>();
        }
        ImGui::EndPopup();
    }

    // 绘制实体列表
    m_activeScene->GetRegistry().each([&](auto entityId) {
        Entity entity{ entityId, m_activeScene.get() };
        DrawEntityNode(entity);
    });

    // 点击空白区域取消选择
    if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
        m_selectedEntity = {};
    }

    ImGui::End();
}

void EditorLayer::DrawEntityNode(Entity entity) {
    auto& tag = entity.GetComponent<TagComponent>().tag;

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                               ImGuiTreeNodeFlags_SpanAvailWidth;

    if (m_selectedEntity == entity) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    // 没有子实体时显示为叶节点
    flags |= ImGuiTreeNodeFlags_Leaf;

    bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity,
                                     flags, "%s", tag.c_str());

    // 选择
    if (ImGui::IsItemClicked()) {
        m_selectedEntity = entity;
    }

    // 右键菜单
    bool entityDeleted = false;
    if (ImGui::BeginPopupContextItem()) {
        if (ImGui::MenuItem("Delete Entity")) {
            entityDeleted = true;
        }
        if (ImGui::MenuItem("Duplicate Entity")) {
            // 实现复制逻辑
        }
        ImGui::EndPopup();
    }

    if (opened) {
        // 绘制子实体 (如果实现了层级关系)
        ImGui::TreePop();
    }

    if (entityDeleted) {
        if (m_selectedEntity == entity) {
            m_selectedEntity = {};
        }
        m_activeScene->DestroyEntity(entity);
    }
}
```

### 18.4 Inspector Panel

```cpp
// ============ 检视器面板 ============

void EditorLayer::DrawInspectorPanel() {
    ImGui::Begin("Inspector");

    if (m_selectedEntity) {
        DrawComponents(m_selectedEntity);

        ImGui::Separator();

        // 添加组件按钮
        ImGui::PushItemWidth(-1);
        if (ImGui::Button("Add Component", ImVec2(-1, 0))) {
            ImGui::OpenPopup("AddComponent");
        }

        if (ImGui::BeginPopup("AddComponent")) {
            DrawAddComponentMenu();
            ImGui::EndPopup();
        }
        ImGui::PopItemWidth();
    }

    ImGui::End();
}

void EditorLayer::DrawComponents(Entity entity) {
    // Tag Component
    if (entity.HasComponent<TagComponent>()) {
        auto& tag = entity.GetComponent<TagComponent>().tag;

        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        strncpy(buffer, tag.c_str(), sizeof(buffer) - 1);

        if (ImGui::InputText("##Tag", buffer, sizeof(buffer))) {
            tag = std::string(buffer);
        }
    }

    ImGui::SameLine();
    ImGui::PushItemWidth(-1);

    // 实体是否激活
    // ImGui::Checkbox("##Active", &entity.IsActive());

    ImGui::PopItemWidth();

    ImGui::Separator();

    // Transform Component
    DrawComponent<TransformComponent>("Transform", entity, [](auto& component) {
        DrawVec3Control("Position", component.translation);
        glm::vec3 rotation = glm::degrees(component.rotation);
        DrawVec3Control("Rotation", rotation);
        component.rotation = glm::radians(rotation);
        DrawVec3Control("Scale", component.scale, 1.0f);
    });

    // Camera Component
    DrawComponent<CameraComponent>("Camera", entity, [](auto& component) {
        auto& camera = component.camera;

        ImGui::Checkbox("Primary", &component.primary);

        const char* projectionTypes[] = { "Perspective", "Orthographic" };
        int currentProjection = (int)camera.GetProjectionType();

        if (ImGui::Combo("Projection", &currentProjection, projectionTypes, 2)) {
            camera.SetProjectionType((SceneCamera::ProjectionType)currentProjection);
        }

        if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective) {
            float fov = glm::degrees(camera.GetPerspectiveVerticalFOV());
            if (ImGui::DragFloat("FOV", &fov, 0.1f, 1.0f, 179.0f)) {
                camera.SetPerspectiveVerticalFOV(glm::radians(fov));
            }
            float nearClip = camera.GetPerspectiveNearClip();
            if (ImGui::DragFloat("Near", &nearClip, 0.01f)) {
                camera.SetPerspectiveNearClip(nearClip);
            }
            float farClip = camera.GetPerspectiveFarClip();
            if (ImGui::DragFloat("Far", &farClip, 1.0f)) {
                camera.SetPerspectiveFarClip(farClip);
            }
        }
    });

    // Mesh Renderer Component
    DrawComponent<MeshRendererComponent>("Mesh Renderer", entity, [](auto& component) {
        ImGui::Checkbox("Visible", &component.visible);
        ImGui::Checkbox("Cast Shadow", &component.castShadow);
        ImGui::Checkbox("Receive Shadow", &component.receiveShadow);

        // 材质列表
        if (component.mesh && component.mesh->GetMaterialTable()) {
            auto& materials = component.mesh->GetMaterialTable();
            for (uint32_t i = 0; i < materials->GetSlotCount(); i++) {
                ImGui::PushID(i);
                ImGui::Text("Slot %d:", i);
                // 材质选择 UI
                ImGui::PopID();
            }
        }
    });

    // Directional Light Component
    DrawComponent<DirectionalLightComponent>("Directional Light", entity, [](auto& component) {
        ImGui::ColorEdit3("Color", glm::value_ptr(component.color));
        ImGui::DragFloat("Intensity", &component.intensity, 0.01f, 0.0f, 100.0f);
        ImGui::Checkbox("Cast Shadow", &component.castShadow);
    });

    // Point Light Component
    DrawComponent<PointLightComponent>("Point Light", entity, [](auto& component) {
        ImGui::ColorEdit3("Color", glm::value_ptr(component.color));
        ImGui::DragFloat("Intensity", &component.intensity, 0.01f, 0.0f, 100.0f);
        ImGui::DragFloat("Radius", &component.radius, 0.1f, 0.0f, 1000.0f);
    });
}

// 通用组件绘制模板
template<typename T, typename UIFunction>
void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction) {
    if (!entity.HasComponent<T>()) return;

    const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen |
                                     ImGuiTreeNodeFlags_Framed |
                                     ImGuiTreeNodeFlags_AllowItemOverlap |
                                     ImGuiTreeNodeFlags_SpanAvailWidth |
                                     ImGuiTreeNodeFlags_FramePadding;

    auto& component = entity.GetComponent<T>();
    ImVec2 contentRegionAvail = ImGui::GetContentRegionAvail();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
    float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
    ImGui::Separator();
    bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), flags, "%s", name.c_str());
    ImGui::PopStyleVar();

    // 组件设置按钮
    ImGui::SameLine(contentRegionAvail.x - lineHeight * 0.5f);
    if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight })) {
        ImGui::OpenPopup("ComponentSettings");
    }

    bool removeComponent = false;
    if (ImGui::BeginPopup("ComponentSettings")) {
        if (ImGui::MenuItem("Remove Component")) {
            removeComponent = true;
        }
        ImGui::EndPopup();
    }

    if (open) {
        uiFunction(component);
        ImGui::TreePop();
    }

    if (removeComponent) {
        entity.RemoveComponent<T>();
    }
}
```

### 18.5 Viewport 与 Gizmo

```cpp
// ============ 视口面板 ============

void EditorLayer::DrawViewportPanel() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
    ImGui::Begin("Viewport");

    auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
    auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
    auto viewportOffset = ImGui::GetWindowPos();
    m_viewportBounds[0] = { viewportMinRegion.x + viewportOffset.x,
                           viewportMinRegion.y + viewportOffset.y };
    m_viewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x,
                           viewportMaxRegion.y + viewportOffset.y };

    m_viewportFocused = ImGui::IsWindowFocused();
    m_viewportHovered = ImGui::IsWindowHovered();

    // 阻止事件传递
    Application::Get().GetImGuiLayer()->BlockEvents(!m_viewportFocused && !m_viewportHovered);

    ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
    m_viewportSize = { viewportPanelSize.x, viewportPanelSize.y };

    // 渲染视口
    uint64_t textureID = m_viewportFramebuffer->GetColorAttachmentRendererID();
    ImGui::Image((void*)textureID, viewportPanelSize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

    // 拖放目标
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM")) {
            const wchar_t* path = (const wchar_t*)payload->Data;
            // 处理拖放的文件
            HandleDroppedFile(std::filesystem::path(path));
        }
        ImGui::EndDragDropTarget();
    }

    // Gizmo
    DrawGizmos();

    ImGui::End();
    ImGui::PopStyleVar();
}

void EditorLayer::DrawGizmos() {
    if (!m_selectedEntity || m_gizmoType == -1) return;

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    ImGuizmo::SetRect(m_viewportBounds[0].x, m_viewportBounds[0].y,
                      m_viewportBounds[1].x - m_viewportBounds[0].x,
                      m_viewportBounds[1].y - m_viewportBounds[0].y);

    // 相机矩阵
    const glm::mat4& cameraProjection = m_editorCamera.GetProjection();
    glm::mat4 cameraView = m_editorCamera.GetViewMatrix();

    // 实体变换
    auto& tc = m_selectedEntity.GetComponent<TransformComponent>();
    glm::mat4 transform = tc.GetTransform();

    // Snap
    bool snap = Input::IsKeyPressed(Key::LeftControl);
    float snapValue = 0.5f;  // 位置 snap
    if (m_gizmoType == ImGuizmo::OPERATION::ROTATE) {
        snapValue = 45.0f;   // 旋转 snap (角度)
    }
    float snapValues[3] = { snapValue, snapValue, snapValue };

    // 操作
    ImGuizmo::Manipulate(glm::value_ptr(cameraView),
                         glm::value_ptr(cameraProjection),
                         (ImGuizmo::OPERATION)m_gizmoType,
                         ImGuizmo::LOCAL,
                         glm::value_ptr(transform),
                         nullptr,
                         snap ? snapValues : nullptr);

    if (ImGuizmo::IsUsing()) {
        glm::vec3 translation, rotation, scale;
        Math::DecomposeTransform(transform, translation, rotation, scale);

        glm::vec3 deltaRotation = rotation - tc.rotation;
        tc.translation = translation;
        tc.rotation += deltaRotation;
        tc.scale = scale;
    }
}
```

### 18.6 快捷键系统

```cpp
bool EditorLayer::OnKeyPressed(KeyPressedEvent& e) {
    // 不处理重复按键
    if (e.IsRepeat()) return false;

    bool control = Input::IsKeyPressed(Key::LeftControl) ||
                   Input::IsKeyPressed(Key::RightControl);
    bool shift = Input::IsKeyPressed(Key::LeftShift) ||
                 Input::IsKeyPressed(Key::RightShift);

    switch (e.GetKeyCode()) {
        // 文件操作
        case Key::N:
            if (control) NewScene();
            break;
        case Key::O:
            if (control) OpenScene();
            break;
        case Key::S:
            if (control) {
                if (shift) SaveSceneAs();
                else SaveScene();
            }
            break;

        // 实体操作
        case Key::D:
            if (control && m_selectedEntity) {
                // 复制实体
            }
            break;
        case Key::Delete:
            if (m_selectedEntity) {
                m_activeScene->DestroyEntity(m_selectedEntity);
                m_selectedEntity = {};
            }
            break;

        // Gizmo 切换
        case Key::Q:
            if (!ImGuizmo::IsUsing()) m_gizmoType = -1;  // 无
            break;
        case Key::W:
            if (!ImGuizmo::IsUsing()) m_gizmoType = ImGuizmo::OPERATION::TRANSLATE;
            break;
        case Key::E:
            if (!ImGuizmo::IsUsing()) m_gizmoType = ImGuizmo::OPERATION::ROTATE;
            break;
        case Key::R:
            if (!ImGuizmo::IsUsing()) m_gizmoType = ImGuizmo::OPERATION::SCALE;
            break;

        // 视图
        case Key::F:
            if (m_selectedEntity) {
                // 聚焦到选中实体
                auto& tc = m_selectedEntity.GetComponent<TransformComponent>();
                m_editorCamera.Focus(tc.translation);
            }
            break;
    }

    return false;
}
```

---

## 19. 序列化系统

### 19.1 为什么选择 YAML？

| 格式 | 优点 | 缺点 |
|------|------|------|
| JSON | 广泛支持、解析快 | 不支持注释、可读性一般 |
| YAML | 高可读性、支持注释 | 解析稍慢、缩进敏感 |
| Binary | 最快、最小 | 不可读、调试困难 |
| XML | 自描述、支持Schema | 冗余、文件大 |

**推荐**：开发期使用 YAML，发布时可转换为二进制

### 19.2 Scene 序列化

```cpp
// ============ src/serialization/scene_serializer.h ============

#pragma once
#include "scene/scene.h"
#include <yaml-cpp/yaml.h>

namespace Mint {

    class SceneSerializer {
    public:
        SceneSerializer(const Ref<Scene>& scene);

        void Serialize(const std::filesystem::path& filepath);
        void SerializeRuntime(const std::filesystem::path& filepath);  // 二进制

        bool Deserialize(const std::filesystem::path& filepath);
        bool DeserializeRuntime(const std::filesystem::path& filepath);

    private:
        void SerializeEntity(YAML::Emitter& out, Entity entity);
        void DeserializeEntity(YAML::Node& entityNode);

    private:
        Ref<Scene> m_scene;
    };

} // namespace Mint
```

```cpp
// ============ src/serialization/scene_serializer.cpp ============

#include "serialization/scene_serializer.h"
#include <fstream>

namespace YAML {
    // glm::vec3 序列化
    template<>
    struct convert<glm::vec3> {
        static Node encode(const glm::vec3& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs) {
            if (!node.IsSequence() || node.size() != 3) return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    // glm::vec4 序列化
    template<>
    struct convert<glm::vec4> {
        static Node encode(const glm::vec4& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs) {
            if (!node.IsSequence() || node.size() != 4) return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };
}

namespace Mint {

    // YAML Emitter 操作符重载
    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v) {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
        : m_scene(scene) {}

    void SceneSerializer::Serialize(const std::filesystem::path& filepath) {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Untitled";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        m_scene->GetRegistry().each([&](auto entityId) {
            Entity entity = { entityId, m_scene.get() };
            if (!entity) return;
            SerializeEntity(out, entity);
        });

        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    void SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity entity) {
        out << YAML::BeginMap;  // Entity

        out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

        // Tag Component
        if (entity.HasComponent<TagComponent>()) {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap;
            auto& tag = entity.GetComponent<TagComponent>().tag;
            out << YAML::Key << "Tag" << YAML::Value << tag;
            out << YAML::EndMap;
        }

        // Transform Component
        if (entity.HasComponent<TransformComponent>()) {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap;
            auto& tc = entity.GetComponent<TransformComponent>();
            out << YAML::Key << "Translation" << YAML::Value << tc.translation;
            out << YAML::Key << "Rotation" << YAML::Value << tc.rotation;
            out << YAML::Key << "Scale" << YAML::Value << tc.scale;
            out << YAML::EndMap;
        }

        // Camera Component
        if (entity.HasComponent<CameraComponent>()) {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap;
            auto& cc = entity.GetComponent<CameraComponent>();
            auto& camera = cc.camera;

            out << YAML::Key << "Camera" << YAML::Value;
            out << YAML::BeginMap;
            out << YAML::Key << "ProjectionType" << YAML::Value
                << (int)camera.GetProjectionType();
            out << YAML::Key << "PerspectiveFOV" << YAML::Value
                << camera.GetPerspectiveVerticalFOV();
            out << YAML::Key << "PerspectiveNear" << YAML::Value
                << camera.GetPerspectiveNearClip();
            out << YAML::Key << "PerspectiveFar" << YAML::Value
                << camera.GetPerspectiveFarClip();
            out << YAML::Key << "OrthographicSize" << YAML::Value
                << camera.GetOrthographicSize();
            out << YAML::Key << "OrthographicNear" << YAML::Value
                << camera.GetOrthographicNearClip();
            out << YAML::Key << "OrthographicFar" << YAML::Value
                << camera.GetOrthographicFarClip();
            out << YAML::EndMap;

            out << YAML::Key << "Primary" << YAML::Value << cc.primary;
            out << YAML::Key << "FixedAspectRatio" << YAML::Value << cc.fixedAspectRatio;
            out << YAML::EndMap;
        }

        // Mesh Renderer Component
        if (entity.HasComponent<MeshRendererComponent>()) {
            out << YAML::Key << "MeshRendererComponent";
            out << YAML::BeginMap;
            auto& mrc = entity.GetComponent<MeshRendererComponent>();
            out << YAML::Key << "Visible" << YAML::Value << mrc.visible;
            out << YAML::Key << "CastShadow" << YAML::Value << mrc.castShadow;
            // 序列化 Mesh 资源路径
            if (mrc.mesh && mrc.mesh->GetResource()) {
                out << YAML::Key << "MeshPath" << YAML::Value
                    << mrc.mesh->GetResource()->GetPath().string();
            }
            out << YAML::EndMap;
        }

        // Directional Light Component
        if (entity.HasComponent<DirectionalLightComponent>()) {
            out << YAML::Key << "DirectionalLightComponent";
            out << YAML::BeginMap;
            auto& lc = entity.GetComponent<DirectionalLightComponent>();
            out << YAML::Key << "Color" << YAML::Value << lc.color;
            out << YAML::Key << "Intensity" << YAML::Value << lc.intensity;
            out << YAML::Key << "CastShadow" << YAML::Value << lc.castShadow;
            out << YAML::EndMap;
        }

        // Point Light Component
        if (entity.HasComponent<PointLightComponent>()) {
            out << YAML::Key << "PointLightComponent";
            out << YAML::BeginMap;
            auto& lc = entity.GetComponent<PointLightComponent>();
            out << YAML::Key << "Color" << YAML::Value << lc.color;
            out << YAML::Key << "Intensity" << YAML::Value << lc.intensity;
            out << YAML::Key << "Radius" << YAML::Value << lc.radius;
            out << YAML::EndMap;
        }

        out << YAML::EndMap;  // Entity
    }

    bool SceneSerializer::Deserialize(const std::filesystem::path& filepath) {
        YAML::Node data;
        try {
            data = YAML::LoadFile(filepath.string());
        } catch (YAML::ParserException& e) {
            MINT_ERROR("Failed to load scene file '{}': {}", filepath.string(), e.what());
            return false;
        }

        if (!data["Scene"]) return false;

        std::string sceneName = data["Scene"].as<std::string>();
        MINT_INFO("Deserializing scene '{}'", sceneName);

        auto entities = data["Entities"];
        if (entities) {
            for (auto entityNode : entities) {
                DeserializeEntity(entityNode);
            }
        }

        return true;
    }

    void SceneSerializer::DeserializeEntity(YAML::Node& entityNode) {
        uint64_t uuid = entityNode["Entity"].as<uint64_t>();

        std::string name;
        auto tagComponent = entityNode["TagComponent"];
        if (tagComponent) {
            name = tagComponent["Tag"].as<std::string>();
        }

        Entity entity = m_scene->CreateEntityWithUUID(uuid, name);

        // Transform Component
        auto transformComponent = entityNode["TransformComponent"];
        if (transformComponent) {
            auto& tc = entity.GetComponent<TransformComponent>();
            tc.translation = transformComponent["Translation"].as<glm::vec3>();
            tc.rotation = transformComponent["Rotation"].as<glm::vec3>();
            tc.scale = transformComponent["Scale"].as<glm::vec3>();
        }

        // Camera Component
        auto cameraComponent = entityNode["CameraComponent"];
        if (cameraComponent) {
            auto& cc = entity.AddComponent<CameraComponent>();
            auto& cameraProps = cameraComponent["Camera"];

            cc.camera.SetProjectionType((SceneCamera::ProjectionType)
                cameraProps["ProjectionType"].as<int>());

            cc.camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
            cc.camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
            cc.camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

            cc.camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
            cc.camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
            cc.camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

            cc.primary = cameraComponent["Primary"].as<bool>();
            cc.fixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
        }

        // 其他组件...
    }

} // namespace Mint
```

---

## 20. 项目架构建议

### 20.1 推荐的目录结构

```
MintEngine/
├── engine/                    # 引擎核心代码
│   ├── src/
│   │   ├── core/              # 核心工具
│   │   │   ├── ref.h          # 智能指针
│   │   │   ├── uuid.h         # UUID
│   │   │   ├── log.h          # 日志
│   │   │   ├── assert.h       # 断言
│   │   │   └── math/          # 数学库
│   │   │
│   │   ├── platform/          # 平台抽象
│   │   │   ├── window.h
│   │   │   ├── input.h
│   │   │   └── file_system.h
│   │   │
│   │   ├── render/            # 渲染系统
│   │   │   ├── rhi/           # RHI 抽象层
│   │   │   │   ├── buffer.h
│   │   │   │   ├── shader.h
│   │   │   │   ├── texture.h
│   │   │   │   └── pipeline.h
│   │   │   │
│   │   │   ├── renderer/      # 渲染器
│   │   │   │   ├── render_pass.h
│   │   │   │   ├── forward_renderer.h
│   │   │   │   └── post_process.h
│   │   │   │
│   │   │   ├── material.h
│   │   │   ├── mesh.h
│   │   │   ├── light.h
│   │   │   └── camera.h
│   │   │
│   │   ├── scene/             # 场景系统
│   │   │   ├── scene.h
│   │   │   ├── entity.h
│   │   │   └── components.h
│   │   │
│   │   ├── asset/             # 资源系统
│   │   │   ├── asset.h
│   │   │   ├── asset_manager.h
│   │   │   └── importers/
│   │   │
│   │   ├── animation/         # 动画系统
│   │   │   ├── skeleton.h
│   │   │   ├── animation_clip.h
│   │   │   └── animator.h
│   │   │
│   │   ├── audio/             # 音频系统 (未来)
│   │   ├── physics/           # 物理系统 (未来)
│   │   └── scripting/         # 脚本系统 (未来)
│   │
│   └── vendor/                # 第三方库
│       ├── glfw/
│       ├── glad/
│       ├── imgui/
│       ├── entt/
│       └── ...
│
├── editor/                    # 编辑器
│   ├── src/
│   │   ├── editor_layer.h
│   │   ├── panels/
│   │   │   ├── scene_hierarchy.h
│   │   │   ├── inspector.h
│   │   │   └── content_browser.h
│   │   └── ...
│   └── assets/                # 编辑器资源
│       ├── icons/
│       └── fonts/
│
├── sandbox/                   # 测试项目
│   ├── src/
│   │   └── sandbox.cpp
│   └── assets/
│       ├── shaders/
│       ├── textures/
│       └── models/
│
├── docs/                      # 文档
├── scripts/                   # 构建脚本
└── CMakeLists.txt
```

### 20.2 模块依赖图

```
┌─────────────────────────────────────────────────────────────┐
│                        Application                           │
│                     (Editor / Sandbox)                       │
└──────────────────────────┬──────────────────────────────────┘
                           │
┌──────────────────────────▼──────────────────────────────────┐
│                         Scene                                │
│              (Entity, Components, SceneGraph)                │
└────────────┬─────────────────────────────────┬──────────────┘
             │                                 │
┌────────────▼────────────┐     ┌──────────────▼──────────────┐
│        Renderer         │     │        Animation            │
│  (RenderPass, Material) │     │  (Skeleton, Animator)       │
└────────────┬────────────┘     └──────────────┬──────────────┘
             │                                 │
┌────────────▼────────────────────────────────▼───────────────┐
│                          Asset                               │
│               (AssetManager, Importers)                      │
└──────────────────────────┬──────────────────────────────────┘
                           │
┌──────────────────────────▼──────────────────────────────────┐
│                          RHI                                 │
│           (Buffer, Texture, Shader, Pipeline)                │
└──────────────────────────┬──────────────────────────────────┘
                           │
┌──────────────────────────▼──────────────────────────────────┐
│                        Platform                              │
│              (Window, Input, FileSystem)                     │
└──────────────────────────┬──────────────────────────────────┘
                           │
┌──────────────────────────▼──────────────────────────────────┐
│                          Core                                │
│            (Ref, UUID, Log, Math, Assert)                    │
└─────────────────────────────────────────────────────────────┘
```

---

## 21. 引擎开发常见陷阱

### 21.1 设计陷阱

| 陷阱 | 描述 | 解决方案 |
|------|------|----------|
| **过早优化** | 在功能完成前就追求性能 | 先实现正确，再优化 |
| **过度抽象** | 为不存在的需求设计接口 | YAGNI 原则 |
| **God Class** | 一个类承担太多职责 | 单一职责原则 |
| **循环依赖** | 模块间互相依赖 | 依赖注入、事件系统 |
| **全局状态** | 过多使用单例和全局变量 | 依赖注入、上下文对象 |

### 21.2 渲染陷阱

| 陷阱 | 描述 | 解决方案 |
|------|------|----------|
| **忘记解绑** | Framebuffer/VAO 未解绑 | RAII 封装 |
| **状态泄漏** | OpenGL 状态未重置 | 显式设置所有状态 |
| **深度精度** | 远近裁剪面设置不当 | 调整近裁剪面、对数深度 |
| **Gamma 错误** | 忽略 sRGB 转换 | 使用 sRGB 纹理格式 |
| **对齐问题** | UBO std140 对齐错误 | 使用 alignas |

### 21.3 内存陷阱

| 陷阱 | 描述 | 解决方案 |
|------|------|----------|
| **循环引用** | 智能指针循环引用 | weak_ptr、手动打破循环 |
| **悬垂指针** | 引用已释放的资源 | 引用计数、生命周期管理 |
| **内存碎片** | 频繁小分配 | 对象池、内存池 |
| **缓存未命中** | 数据布局不连续 | Data-Oriented Design |

---

## 22. 学习路线图

### 22.1 初级阶段 (1-3个月)

```
1. OpenGL 基础
   - 渲染管线
   - 顶点缓冲、着色器
   - 纹理映射
   - 资源: LearnOpenGL.com

2. 基础数学
   - 向量、矩阵
   - 变换、投影
   - 四元数
   - 资源: 3Blue1Brown, GAMES101

3. 引擎基础框架
   - 窗口、事件
   - 输入处理
   - 简单渲染循环
```

### 22.2 中级阶段 (3-6个月)

```
4. 高级渲染
   - 光照模型 (Phong/PBR)
   - 阴影映射
   - 后处理
   - 延迟渲染

5. 资源管理
   - Asset 系统
   - 模型加载 (Assimp)
   - 材质系统

6. 场景管理
   - ECS 系统
   - 场景图
   - 序列化
```

### 22.3 高级阶段 (6-12个月)

```
7. 高级图形
   - Global Illumination
   - Volumetric Effects
   - GPU Driven Rendering
   - Ray Tracing (可选)

8. 引擎系统
   - 骨骼动画
   - 物理引擎
   - 音频系统
   - 脚本系统

9. 工具开发
   - 编辑器
   - 资源管道
   - 调试工具
```

### 22.4 推荐学习资源

**视频课程**：
- GAMES101 - 现代图形学入门
- GAMES104 - 现代游戏引擎
- GAMES202 - 高质量实时渲染
- TheCherno - Game Engine Series

**书籍**：
- 《Real-Time Rendering》
- 《Game Engine Architecture》
- 《Physically Based Rendering》
- 《GPU Gems》系列

**开源项目**：
- Hazel Engine - 最佳学习项目
- Piccolo Engine - 国产教学引擎
- Godot - 完整开源引擎
- Filament - Google PBR 渲染器

---

*本文档为 MintEngine 开发指南的第三部分。结合 Part 1 和 Part 2 一起阅读，获得完整的引擎开发知识。*

*祝你的引擎开发顺利！*
