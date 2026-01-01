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

current task: reconstruct renderer, 具体步骤见render_command里的注释
相机可能也不能只接收宽高比, 另外, 屏幕缩放也会导致相机的zoom level或者别的成员变量变化, 确定zoom level的取值(SetViewport函数, 需要确认调用时机, 比如缩放事件时, editor camera的OnEvent, 或者未来转交给editor进行处理？editor layer通过OnUpdate, 处理Scene, Framebuffer, editor_camera的resize
next: 创建framebuffer内容, 修正viewport调整

- 实现FrameBuffer前考虑对ImageFormat相关的抽象