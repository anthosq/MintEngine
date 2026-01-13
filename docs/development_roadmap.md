# MintEngine 游戏引擎开发计划

> MintEngine 目标：一个完整的、模块化的游戏引擎，支持3D游戏开发

---

## 引擎架构总览

```
┌─────────────────────────────────────────────────────────────────────────┐
│                           Game / Editor                                  │
├─────────────────────────────────────────────────────────────────────────┤
│                           Scripting Layer                                │
│                        (Lua / C# / Native C++)                          │
├───────────┬───────────┬───────────┬───────────┬───────────┬─────────────┤
│  Scene    │  Render   │  Physics  │  Audio    │  Input    │    AI       │
│  System   │  System   │  System   │  System   │  System   │  System     │
├───────────┴───────────┴───────────┴───────────┴───────────┴─────────────┤
│                           Asset System                                   │
├─────────────────────────────────────────────────────────────────────────┤
│                           Core Layer                                     │
│              (Memory, Threading, Math, Events, Logging)                  │
├─────────────────────────────────────────────────────────────────────────┤
│                           Platform Layer                                 │
│                    (Window, FileSystem, Timer)                           │
└─────────────────────────────────────────────────────────────────────────┘
```

---

## 项目现状

### 已完成
- 平台层: 窗口系统(GLFW)、事件系统、日志系统(spdlog)
- 核心层: 引用计数智能指针 `Ref<T>`、时间步
- 渲染层: OpenGL基础(缓冲、纹理、着色器)、编辑器相机、ImGui

### 进行中
- Mesh系统、渲染命令队列、着色器反射、Framebuffer

### 待开发
- 完整渲染管线、物理系统、音频系统、脚本系统、AI系统、动画系统、UI系统、编辑器

---

## 开发阶段总览 (渲染+物理优先路线)

| 优先级 | 阶段 | 内容 | 状态 |
|--------|------|------|------|
| **P0** | Stage 1 | 核心基础设施 | 🔴 待开始 |
| **P0** | Stage 2 | 渲染系统完善 | 🟡 进行中 |
| **P0** | Stage 3 | 场景系统 (ECS) | 🔴 待开始 |
| **P1** | Stage 4 | 物理系统 (Jolt) | 🔴 待开始 |
| **P2** | Stage 5 | 动画系统 | 🔴 待开始 |
| **P3** | Stage 6 | 音频系统 | 🔴 待开始 |
| **P2** | Stage 7 | 脚本系统 (Lua) | 🔴 待开始 |
| **P3** | Stage 8 | UI系统 | 🔴 待开始 |
| **P4** | Stage 9 | AI系统 | 🔴 待开始 |
| **P2** | Stage 10 | 编辑器 | 🔴 待开始 |

**技术选型**: Jolt Physics + Lua 脚本

---

# Stage 1: 核心基础设施

## 1.1 内存管理系统
**目录**: `src/core/memory/`

```cpp
// 线性分配器 - 用于帧临时数据
class LinearAllocator {
    void* Allocate(size_t size, size_t alignment = 16);
    void Reset();  // 每帧重置
};

// 池分配器 - 用于固定大小对象
template<typename T>
class PoolAllocator {
    T* Allocate();
    void Free(T* ptr);
};

// 栈分配器 - LIFO 分配
class StackAllocator {
    void* Allocate(size_t size);
    void Free(void* ptr);  // 必须按顺序释放
};
```

**为什么需要**: 游戏引擎需要高效内存管理，减少碎片和分配开销

---

## 1.2 多线程系统
**目录**: `src/core/threading/`

```cpp
// 任务系统
class JobSystem {
    void Initialize(uint32_t numWorkers);
    void Shutdown();

    JobHandle Schedule(Job&& job);
    void Wait(JobHandle handle);
    void WaitAll();
};

// 线程安全容器
template<typename T>
class ConcurrentQueue {
    void Push(T&& item);
    bool TryPop(T& item);
};
```

**应用场景**: 资源异步加载、物理模拟、AI寻路

---

## 1.3 Asset 资源系统
**目录**: `src/asset/`

```cpp
enum class AssetType {
    None, Texture2D, TextureCube, Shader, Material,
    Mesh, Animation, Audio, Scene, Prefab, Script
};

class AssetManager {
    // 同步加载
    template<typename T> Ref<T> Load(const path& path);

    // 异步加载
    template<typename T> AssetFuture<T> LoadAsync(const path& path);

    // 热重载支持
    void ReloadAsset(AssetHandle handle);
    void WatchForChanges(bool enable);
};
```

---

# Stage 2: 渲染系统

## 2.1 RHI 抽象层
**目录**: `src/render/rhi/`

```cpp
// 渲染硬件接口 - 支持多后端
class RHIDevice {
    virtual Ref<RHIBuffer> CreateBuffer(const BufferDesc& desc) = 0;
    virtual Ref<RHITexture> CreateTexture(const TextureDesc& desc) = 0;
    virtual Ref<RHIPipeline> CreatePipeline(const PipelineDesc& desc) = 0;
};

// Pipeline State Object
struct PipelineDesc {
    Ref<Shader> vertexShader;
    Ref<Shader> fragmentShader;
    BlendState blendState;
    DepthStencilState depthState;
    RasterState rasterState;
    VertexLayout vertexLayout;
};
```

## 2.2 材质系统
**目录**: `src/render/material/`

```cpp
class Material : public Asset {
    void Set(const std::string& name, auto value);
    void SetTexture(const std::string& name, Ref<Texture> tex);
    void Bind();

    Ref<Shader> GetShader();
    MaterialFlag GetFlags();
};

class MaterialInstance {
    Ref<Material> m_baseMaterial;
    Buffer m_overrideData;
};
```

## 2.3 Mesh 系统
**目录**: `src/render/mesh/`

- MeshResource: 共享的GPU数据
- StaticMesh: 场景中的实例
- SkeletalMesh: 带骨骼的网格

## 2.4 光照系统
**目录**: `src/render/lighting/`

- DirectionalLight, PointLight, SpotLight, AreaLight
- LightEnvironment: 管理场景光源
- ShadowRenderer: 阴影映射

## 2.5 RenderPass 系统
**目录**: `src/render/passes/`

```cpp
class RenderPass {
    virtual void Setup(RenderGraph& graph) = 0;
    virtual void Execute(RenderContext& ctx) = 0;
};

// 预定义 Pass
class ShadowPass : public RenderPass;
class GBufferPass : public RenderPass;    // 延迟渲染
class ForwardPass : public RenderPass;
class SkyboxPass : public RenderPass;
class PostProcessPass : public RenderPass;
```

## 2.6 后处理系统
**目录**: `src/render/postprocess/`

- Bloom, ToneMapping, FXAA/TAA, DOF, MotionBlur, SSAO

---

# Stage 3: 场景系统

## 3.1 ECS 框架
**目录**: `src/scene/`
**依赖**: entt

```cpp
class Scene {
    Entity CreateEntity(const std::string& name);
    void DestroyEntity(Entity entity);

    void OnUpdate(TimeStep ts);
    void OnPhysicsUpdate(TimeStep fixedTs);
    void OnRender(Camera& camera);

    template<typename... Components>
    auto View() { return m_registry.view<Components...>(); }
};
```

## 3.2 核心组件
```cpp
// 基础
struct IDComponent { UUID id; };
struct TagComponent { std::string tag; };
struct TransformComponent { vec3 position, rotation, scale; };
struct HierarchyComponent { Entity parent; std::vector<Entity> children; };

// 渲染
struct MeshRendererComponent { Ref<StaticMesh> mesh; };
struct SkeletalMeshComponent { Ref<SkeletalMesh> mesh; Ref<Animator> animator; };
struct CameraComponent { SceneCamera camera; bool primary; };
struct LightComponent { LightType type; LightData data; };

// 物理 (Stage 4)
struct RigidbodyComponent { ... };
struct ColliderComponent { ... };

// 音频 (Stage 6)
struct AudioSourceComponent { ... };
struct AudioListenerComponent { ... };

// 脚本 (Stage 7)
struct ScriptComponent { ... };
```

## 3.3 Prefab 系统
```cpp
class Prefab : public Asset {
    Entity Instantiate(Scene& scene);
    void ApplyChanges(Entity instance);
};
```

## 3.4 场景序列化
**依赖**: yaml-cpp 或 nlohmann/json

---

# Stage 4: 物理系统

## 4.1 物理引擎: Jolt Physics
**选择原因**:
- 现代 C++17 设计，API 清晰
- 高性能，被 Horizon Forbidden West 等 AAA 游戏使用
- MIT 开源协议
- 活跃维护

**集成方式**: 作为 submodule 添加到 `src/3rdparty/JoltPhysics/`

## 4.2 物理抽象层
**目录**: `src/physics/`

```cpp
class PhysicsWorld {
    void Step(float deltaTime);

    RigidbodyHandle CreateRigidbody(const RigidbodyDesc& desc);
    void DestroyRigidbody(RigidbodyHandle handle);

    ColliderHandle CreateCollider(const ColliderDesc& desc);

    bool Raycast(const Ray& ray, RaycastHit& hit);
    std::vector<Collider*> OverlapSphere(vec3 center, float radius);
};

struct RigidbodyDesc {
    BodyType type;  // Static, Kinematic, Dynamic
    float mass;
    float linearDamping;
    float angularDamping;
    bool useGravity;
};

struct ColliderDesc {
    ColliderType type;  // Box, Sphere, Capsule, Mesh
    vec3 size;
    vec3 offset;
    bool isTrigger;
    PhysicsMaterial material;
};
```

## 4.3 碰撞检测
```cpp
// 碰撞回调
class CollisionListener {
    virtual void OnCollisionEnter(Collision& collision);
    virtual void OnCollisionStay(Collision& collision);
    virtual void OnCollisionExit(Collision& collision);
    virtual void OnTriggerEnter(Collider* other);
    virtual void OnTriggerExit(Collider* other);
};
```

---

# Stage 5: 动画系统

## 5.1 骨骼动画
**目录**: `src/animation/`

```cpp
class Skeleton {
    std::vector<Bone> bones;
    void CalculateSkinningMatrices(std::vector<mat4>& matrices);
};

class AnimationClip : public Asset {
    float duration;
    std::vector<BoneTrack> tracks;

    void Sample(float time, Skeleton& skeleton);
};

class Animator {
    void Play(Ref<AnimationClip> clip);
    void CrossFade(Ref<AnimationClip> clip, float duration);
    void Update(float deltaTime);

    const std::vector<mat4>& GetSkinningMatrices();
};
```

## 5.2 动画状态机
```cpp
class AnimationStateMachine {
    void AddState(const std::string& name, Ref<AnimationClip> clip);
    void AddTransition(const std::string& from, const std::string& to,
                       const Condition& condition);
    void SetParameter(const std::string& name, float/int/bool value);
    void Update(float deltaTime);
};
```

## 5.3 混合树 (Blend Tree)
```cpp
class BlendTree {
    // 1D Blend: 根据单一参数混合
    void Add1DBlend(float threshold, Ref<AnimationClip> clip);

    // 2D Blend: 根据两个参数混合 (如移动方向)
    void Add2DBlend(vec2 position, Ref<AnimationClip> clip);
};
```

## 5.4 IK (Inverse Kinematics)
```cpp
class IKSolver {
    void SolveTwoBoneIK(Bone& root, Bone& mid, Bone& tip, vec3 target);
    void SolveFABRIK(std::vector<Bone*>& chain, vec3 target);
};
```

---

# Stage 6: 音频系统

## 6.1 音频引擎选择
| 引擎 | 优点 | 缺点 |
|------|------|------|
| **OpenAL** | 跨平台、开源 | 功能有限 |
| **FMOD** | 专业、功能强 | 商业授权 |
| **Wwise** | AAA级 | 复杂、昂贵 |
| **miniaudio** | 轻量、单头文件 | 功能基础 |

**推荐**: miniaudio (学习) 或 FMOD (专业)

## 6.2 音频抽象层
**目录**: `src/audio/`

```cpp
class AudioEngine {
    void Initialize();
    void Update();
    void Shutdown();

    void SetListenerTransform(const mat4& transform);
};

class AudioSource {
    void Play();
    void Pause();
    void Stop();

    void SetClip(Ref<AudioClip> clip);
    void SetVolume(float volume);
    void SetPitch(float pitch);
    void SetLoop(bool loop);
    void Set3DAttributes(vec3 position, vec3 velocity);
    void SetSpatialBlend(float blend);  // 0=2D, 1=3D
};

class AudioClip : public Asset {
    float duration;
    int channels;
    int sampleRate;
};
```

## 6.3 音频混合器
```cpp
class AudioMixer {
    AudioGroup* CreateGroup(const std::string& name);
    void SetGroupVolume(const std::string& name, float volume);
    void AddEffect(const std::string& group, Ref<AudioEffect> effect);
};

// 音频效果
class ReverbEffect : public AudioEffect;
class LowPassFilter : public AudioEffect;
class DistortionEffect : public AudioEffect;
```

---

# Stage 7: 脚本系统

## 7.1 脚本系统: Lua
**选择原因**:
- 轻量级，易于嵌入
- 热重载支持，无需重新编译
- 游戏设计师友好
- 广泛的游戏行业使用经验

**依赖**: sol2 (推荐) 或 LuaBridge
**集成方式**: `src/3rdparty/lua/` + `src/3rdparty/sol2/`

## 7.2 Lua 脚本引擎
**目录**: `src/scripting/`

```cpp
class LuaScriptEngine {
public:
    void Initialize();
    void Shutdown();

    // 脚本加载
    void LoadScript(const path& filepath);
    void ReloadScript(const path& filepath);  // 热重载

    // 调用 Lua 函数
    void CallFunction(const std::string& name);
    template<typename... Args>
    void CallFunction(const std::string& name, Args&&... args);

    // 暴露 C++ 到 Lua
    void ExposeEngineAPI();

private:
    sol::state m_luaState;
};
```

## 7.3 Lua 脚本示例

```lua
-- player_controller.lua
PlayerController = {}
PlayerController.__index = PlayerController

function PlayerController:new(entity)
    local self = setmetatable({}, PlayerController)
    self.entity = entity
    self.moveSpeed = 5.0
    self.jumpForce = 10.0
    return self
end

function PlayerController:OnUpdate(deltaTime)
    local transform = self.entity:GetTransform()

    -- 移动
    if Input.IsKeyPressed(Key.W) then
        transform.position = transform.position + Vec3(0, 0, -1) * self.moveSpeed * deltaTime
    end

    -- 跳跃
    if Input.IsKeyPressed(Key.Space) and self:IsGrounded() then
        local rb = self.entity:GetRigidbody()
        rb:AddForce(Vec3(0, self.jumpForce, 0), ForceMode.Impulse)
    end
end

function PlayerController:IsGrounded()
    -- 射线检测地面
    return Physics.Raycast(self.entity:GetPosition(), Vec3(0, -1, 0), 0.1)
end
```

## 7.4 C++ 到 Lua 绑定

```cpp
void LuaScriptEngine::ExposeEngineAPI() {
    // 数学类型
    m_luaState.new_usertype<glm::vec3>("Vec3",
        sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(),
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z,
        sol::meta_function::addition, [](const glm::vec3& a, const glm::vec3& b) { return a + b; },
        sol::meta_function::multiplication, [](const glm::vec3& v, float s) { return v * s; }
    );

    // Entity
    m_luaState.new_usertype<Entity>("Entity",
        "GetTransform", &Entity::GetComponent<TransformComponent>,
        "GetRigidbody", &Entity::GetComponent<RigidbodyComponent>,
        "GetPosition", [](Entity& e) { return e.GetComponent<TransformComponent>().position; }
    );

    // Input
    m_luaState["Input"] = m_luaState.create_table_with(
        "IsKeyPressed", &Input::IsKeyPressed,
        "IsMouseButtonPressed", &Input::IsMouseButtonPressed,
        "GetMousePosition", &Input::GetMousePosition
    );

    // Physics
    m_luaState["Physics"] = m_luaState.create_table_with(
        "Raycast", &PhysicsWorld::Raycast,
        "OverlapSphere", &PhysicsWorld::OverlapSphere
    );
}
```

---

# Stage 8: UI 系统

## 8.1 游戏内 UI (非 ImGui)
**目录**: `src/ui/`

```cpp
// UI 元素基类
class UIElement {
    vec2 position;
    vec2 size;
    vec2 anchor;
    vec2 pivot;
    bool visible;

    virtual void OnRender(UIRenderer& renderer) = 0;
    virtual bool OnEvent(Event& event) { return false; }
};

// 常用控件
class UIImage : public UIElement;
class UIText : public UIElement;
class UIButton : public UIElement;
class UISlider : public UIElement;
class UIPanel : public UIElement;
class UIScrollView : public UIElement;

// UI Canvas
class UICanvas {
    void AddElement(Ref<UIElement> element);
    void Render();
    void HandleEvent(Event& event);

    RenderMode renderMode;  // ScreenSpace, WorldSpace
};
```

## 8.2 UI 布局系统
```cpp
// 布局组件
class HorizontalLayout : public UIElement {
    float spacing;
    Alignment alignment;
};

class VerticalLayout : public UIElement;
class GridLayout : public UIElement;
```

## 8.3 UI 渲染器
```cpp
class UIRenderer {
    void DrawRect(vec2 pos, vec2 size, vec4 color);
    void DrawTexture(vec2 pos, vec2 size, Ref<Texture> tex);
    void DrawText(vec2 pos, const std::string& text, Ref<Font> font);
    void DrawNineSlice(vec2 pos, vec2 size, Ref<Texture> tex, vec4 border);
};
```

---

# Stage 9: AI 系统

## 9.1 导航系统
**目录**: `src/ai/navigation/`
**依赖**: Recast/Detour

```cpp
class NavMesh {
    static Ref<NavMesh> Build(const NavMeshBuildSettings& settings);
    bool FindPath(vec3 start, vec3 end, std::vector<vec3>& path);
    vec3 GetClosestPoint(vec3 position);
};

class NavMeshAgent {
    void SetDestination(vec3 target);
    void Stop();

    float speed;
    float acceleration;
    float stoppingDistance;
    float radius;
    float height;

    bool HasPath();
    bool IsAtDestination();
};
```

## 9.2 行为树
**目录**: `src/ai/behavior/`

```cpp
// 节点类型
class BTNode {
    virtual BTStatus Execute(Blackboard& bb) = 0;
};

// 组合节点
class BTSelector : public BTNode;   // 或节点
class BTSequence : public BTNode;   // 与节点
class BTParallel : public BTNode;   // 并行节点

// 装饰节点
class BTInverter : public BTNode;   // 取反
class BTRepeater : public BTNode;   // 重复
class BTSucceeder : public BTNode;  // 总是成功

// 叶节点
class BTAction : public BTNode;     // 执行动作
class BTCondition : public BTNode;  // 检查条件

// 黑板 (共享数据)
class Blackboard {
    template<typename T>
    void Set(const std::string& key, T value);

    template<typename T>
    T Get(const std::string& key);
};
```

## 9.3 感知系统
```cpp
class PerceptionSystem {
    void RegisterStimulus(const Stimulus& stimulus);
    void Update(float deltaTime);
};

class AIPerceptionComponent {
    void AddSense(Ref<AISense> sense);

    // 事件回调
    std::function<void(const Stimulus&)> OnPerceptionUpdated;
};

// 感知类型
class SightSense : public AISense {
    float range;
    float fov;
    bool CanSee(Entity target);
};

class HearingSense : public AISense {
    float range;
    bool CanHear(const Sound& sound);
};
```

---

# Stage 10: 编辑器

## 10.1 编辑器架构
**目录**: `src/editor/`

```cpp
class EditorLayer : public Layer {
    void OnImGuiRender() override;

private:
    // 面板
    SceneHierarchyPanel m_hierarchyPanel;
    InspectorPanel m_inspectorPanel;
    ViewportPanel m_viewportPanel;
    ContentBrowserPanel m_contentBrowser;
    ConsolePanel m_consolePanel;

    // 编辑器状态
    Ref<Scene> m_editorScene;
    Entity m_selectedEntity;
    EditorCamera m_editorCamera;

    // Gizmo
    int m_gizmoType;
};
```

## 10.2 核心面板
- **Scene Hierarchy**: 场景层级树
- **Inspector**: 组件属性编辑
- **Viewport**: 场景视口 + Gizmo
- **Content Browser**: 资源浏览器
- **Console**: 日志输出
- **Animation**: 动画编辑器
- **Material Editor**: 材质编辑器

## 10.3 编辑器工具
```cpp
// Undo/Redo 系统
class UndoRedoSystem {
    void Execute(Ref<Command> command);
    void Undo();
    void Redo();
};

// 拾取系统
class PickingSystem {
    Entity Pick(vec2 mousePos);  // 通过颜色编码或射线
};

// 资源导入器
class AssetImporter {
    void ImportModel(const path& filepath);
    void ImportTexture(const path& filepath);
    void ImportAudio(const path& filepath);
};
```

---

# 依赖关系图

```
                    ┌─────────────┐
                    │   Stage 1   │
                    │ Core/Asset  │
                    └──────┬──────┘
                           │
              ┌────────────┼────────────┐
              ▼            ▼            ▼
        ┌──────────┐ ┌──────────┐ ┌──────────┐
        │ Stage 2  │ │ Stage 4  │ │ Stage 6  │
        │ Render   │ │ Physics  │ │ Audio    │
        └────┬─────┘ └────┬─────┘ └────┬─────┘
             │            │            │
             └────────────┼────────────┘
                          ▼
                    ┌──────────┐
                    │ Stage 3  │
                    │  Scene   │
                    └────┬─────┘
                         │
              ┌──────────┼──────────┐
              ▼          ▼          ▼
        ┌──────────┐┌──────────┐┌──────────┐
        │ Stage 5  ││ Stage 7  ││ Stage 9  │
        │Animation ││ Script   ││   AI     │
        └────┬─────┘└────┬─────┘└────┬─────┘
             │           │           │
             └───────────┼───────────┘
                         ▼
                   ┌──────────┐
                   │ Stage 8  │
                   │   UI     │
                   └────┬─────┘
                        │
                        ▼
                   ┌──────────┐
                   │ Stage 10 │
                   │ Editor   │
                   └──────────┘
```

---

# 第三方库推荐

| 模块 | 库 | 用途 |
|------|-----|------|
| 窗口 | GLFW | 跨平台窗口/输入 |
| 图形 | glad | OpenGL加载 |
| 数学 | glm | 向量/矩阵运算 |
| 模型 | assimp | 模型导入 |
| 图像 | stb_image | 图像加载 |
| 日志 | spdlog | 日志系统 |
| ECS | entt | Entity-Component-System |
| 序列化 | yaml-cpp | YAML解析 |
| 物理 | Jolt/Bullet | 物理模拟 |
| 音频 | miniaudio/FMOD | 音频播放 |
| 导航 | Recast/Detour | NavMesh生成 |
| UI | ImGui | 编辑器UI |
| 脚本 | sol2 | Lua绑定 |
| 字体 | FreeType + msdf | 文本渲染 |

---

# 验证里程碑

## Milestone 1: 基础渲染 Demo
- 加载并渲染3D模型
- 基础光照 (方向光 + 点光源)
- 天空盒
- 简单阴影

## Milestone 2: 物理交互 Demo
- 刚体模拟
- 碰撞检测与响应
- 物理材质 (摩擦、弹性)

## Milestone 3: 角色控制 Demo
- 骨骼动画播放
- 动画状态机
- 第三人称相机
- 基础移动控制

## Milestone 4: 完整游戏 Demo
- 小型3D游戏场景
- NPC + AI行为
- UI界面
- 音效和背景音乐
- 可玩的游戏循环

---

# 推荐学习资源

**书籍**:
- 《Game Engine Architecture》 - Jason Gregory
- 《Real-Time Rendering》 - 渲染圣经
- 《Game Programming Patterns》 - 设计模式
- 《AI for Games》 - 游戏AI

**开源引擎**:
- Hazel Engine - 最佳学习参考
- Godot - 完整开源引擎
- Piccolo - GAMES104 教学引擎

**课程**:
- GAMES101/104/202 - 图形学系列
- TheCherno YouTube - 引擎开发系列
