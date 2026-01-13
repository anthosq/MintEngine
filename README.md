# TODO List
- reconstruct the camera class, preparing for the editor.
- implement the mesh and reconstruct the rendersystem.
  - mesh as a layer, implement the OnUpdate and other methods.
  - (need to reconstrcut renderer, and consider adding animation & mesh) 
- rearrange the file structure:
  - move event, math to core
  - create function folder, contains the render and other function part
  - for core: (not sure)
    - base
      - macro; hash; UUID?
    - math
      - quarterion
      - vec
    - event
    - input
- clean up the sandbox code.


- 实现FrameBuffer前考虑对ImageFormat相关的抽象


## 2026.1.3 开发计划:
- 整理Shader class, 处理Shader Uniform部分
- 处理Material, 初步实现Material Asset, Material RHI
- 处理Mesh
- 后续目标: 天空盒, ECS, 与光照
- 序列化与反射