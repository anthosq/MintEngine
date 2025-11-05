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

current task: reconstruct the camera class, renderer 