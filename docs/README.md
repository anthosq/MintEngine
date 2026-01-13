# MintEngine 开发文档索引

## 文档概览

本文档集为 MintEngine 提供完整的开发指南，涵盖从基础架构到高级渲染的所有主题。

---

## 开发计划

### [游戏引擎开发路线图](./development_roadmap.md)

完整的游戏引擎开发计划，包含 10 个开发阶段：

| 阶段 | 内容 | 优先级 |
|------|------|--------|
| Stage 1 | 核心基础设施 (内存、多线程、资源) | P0 |
| Stage 2 | 渲染系统 (RHI、材质、光照、后处理) | P0 |
| Stage 3 | 场景系统 (ECS、组件、Prefab) | P0 |
| Stage 4 | 物理系统 (Jolt Physics) | P1 |
| Stage 5 | 动画系统 (骨骼、状态机、IK) | P2 |
| Stage 6 | 音频系统 | P3 |
| Stage 7 | 脚本系统 (Lua + sol2) | P2 |
| Stage 8 | UI 系统 | P3 |
| Stage 9 | AI 系统 (NavMesh、行为树) | P4 |
| Stage 10 | 编辑器 | P2 |

---

## 专题深度指南

### [RenderGraph 完全指南](./render_graph_guide.md)

从传统渲染管线的痛点出发，深入理解 RenderGraph 的设计动机与实现：

| 章节 | 内容 |
|------|------|
| 1. 为什么需要 RenderGraph | 传统管线问题、RenderGraph 优势 |
| 2. 核心概念 | 资源句柄、Pass、依赖关系 |
| 3. 设计原理深度解析 | 编译、拓扑排序、资源别名、同步屏障 |
| 4. 完整实现方案 | 核心类定义、实现代码 |
| 5. 使用示例 | 延迟渲染、后处理、条件 Pass |
| 6. 开发规划 | 6 阶段实现路线图 |
| 7. 进阶主题 | 异步计算、多帧资源、调试可视化 |
| 8. 参考资源 | GDC 演讲、开源项目、书籍 |

---

## 技术文档

### [Part 1: 核心架构与基础系统](./engine_development_guide.md)

| 章节 | 内容 |
|------|------|
| 1. 渲染架构总览 | 分层架构设计、职责划分 |
| 2. RenderPass 系统设计 | 继承式 vs 数据驱动、实现时机 |
| 3. Shader 系统与反射机制 | 反射数据结构、OpenGL 反射实现 |
| 4. UniformBuffer 与 GPU 数据管理 | UBO 设计、std140 布局规则 |
| 5. Material 系统设计 | Material 类、MaterialInstance |
| 6. Mesh 系统架构 | Resource/Instance 分离、SubMesh |
| 7. 资源管理系统 | Asset 基类、AssetManager |
| 8. 场景与 ECS 系统 | entt 集成、组件设计 |
| 9. 开发阶段规划 | 完整路线图、阶段任务 |

### [Part 2: 高级渲染与优化](./engine_development_guide_part2.md)

| 章节 | 内容 |
|------|------|
| 10. 光照系统深入设计 | Blinn-Phong、PBR、光源类型 |
| 11. 阴影系统 | Shadow Mapping、PCF 软阴影 |
| 12. 后处理管线 | Bloom、Tone Mapping、FXAA |
| 13. 渲染管线架构 | ForwardRenderer 完整实现 |
| 14. 性能优化策略 | 视锥裁剪、实例化渲染 |
| 15. 调试工具与可视化 | DebugRenderer、性能统计 |
| 16. 常见问题与解决方案 | 渲染问题诊断、调试技巧 |

### [Part 3: 高级系统与工具链](./engine_development_guide_part3.md)

| 章节 | 内容 |
|------|------|
| 17. 骨骼动画系统 | Skeleton、AnimationClip、GPU Skinning |
| 18. 编辑器开发 | EditorLayer、面板实现、Gizmo |
| 19. 序列化系统 | YAML 序列化、Scene 序列化 |
| 20. 项目架构建议 | 目录结构、模块依赖 |
| 21. 引擎开发常见陷阱 | 设计/渲染/内存陷阱 |
| 22. 学习路线图 | 阶段目标、推荐资源 |

---

## 快速参考

### 设计模式使用场景

| 模式 | 使用场景 | 示例 |
|------|----------|------|
| 工厂方法 | 跨平台资源创建 | `Shader::Create()` |
| 模板方法 | RenderPass 流程 | `Begin() → Render() → End()` |
| 策略模式 | 渲染管线切换 | Forward/Deferred |
| 观察者模式 | 事件系统 | Event Dispatcher |
| 组合模式 | ECS 组件 | Entity + Components |

### 标准 Uniform Binding Points

```cpp
namespace UniformBindingPoints {
    constexpr uint32_t Camera    = 0;  // 相机数据
    constexpr uint32_t Transform = 1;  // 变换数据
    constexpr uint32_t Material  = 2;  // 材质数据
    constexpr uint32_t Lighting  = 3;  // 光照数据
    constexpr uint32_t Environment = 4; // 环境数据
    constexpr uint32_t Skinning  = 5;  // 骨骼数据
}
```

### 关键类关系

```
Asset (基类)
├── Texture2D
├── TextureCube
├── Shader
├── Material
├── MeshResource
├── AnimationClip
└── Scene

RefCounter (引用计数基类)
├── Asset
├── Material
├── StaticMesh
├── Animator
└── ...
```

---

## 开发优先级建议

### 阶段 1: 基础设施 (必须先完成)
1. UniformBuffer
2. Shader 反射完善
3. Material 系统

### 阶段 2: 渲染功能
1. RenderPass 系统
2. 光照系统
3. 阴影系统

### 阶段 3: 场景与资源
1. Mesh 系统重构
2. Asset 系统
3. ECS 场景系统

### 阶段 4: 工具链
1. 编辑器
2. 序列化
3. 调试工具

---

## 相关链接

- [LearnOpenGL](https://learnopengl.com/) - OpenGL 教程
- [GAMES101](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html) - 图形学入门
- [GAMES104](https://games104.boomingtech.com/) - 游戏引擎
- [Hazel Engine](https://github.com/TheCherno/Hazel) - 参考项目

---

*更新日期: 2026-01-13*
