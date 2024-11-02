<p align="center"><img width="300" src="./logo.jpg" border="0">
</p>



<p align="center">

<img src="https://img.shields.io/github/issues-pr/Jaysmito101/Xlux?style=for-the-badge" />
<img alt="Lines of code" src="https://img.shields.io/tokei/lines/github/Jaysmito101/Xlux?style=for-the-badge" />
<img src="https://img.shields.io/github/repo-size/Jaysmito101/Xlux?style=for-the-badge" />
<img alt="Maintenance" src="https://img.shields.io/maintenance/yes/2024?style=for-the-badge" />
<a href="https://patreon.com/jaysmito101"><img src="https://img.shields.io/endpoint.svg?url=https%3A%2F%2Fshieldsio-patreon.vercel.app%2Fapi%3Fusername%3Djaysmito101%26type%3Dpledges&style=for-the-badge" alt="Support me on Patreon" /></a>

</p>


# Xlux

Xlux Engine is powerful multi-threaded Software Renderer with *zero* dependencies. It can run on any platform with C++20 available.

Xlux can ideally be used as a library and extended to whatever usage desited with a very flexible API. The forntend of the API is mostly inspited by `vulkan.hpp` but its way simpler compared to the actual vulkan API.

# Features Implemented:

* Cross Platform
* ThreadPool
* Linear Algrebra (Normal)
  - Vector (N dimensional)
  - Matrices (M x N) (and specialized 4x4 for Graphics)
  - SSE/AVX versions of Vector & Matrix (TODO)
* Custom Linear Allocator (for better performance)
* Parallel Shaders API
  - Vertex Stage
  - Fragment Stage
* Xlux Device (resource manager)
* Buffers & Device Memory
* Textures Support
  - Texture 2D
  - Cubemap (TODO)
* Framebuffers
  - Depth Supported
  - Upto 4 (can be expanded) color channels
  - extendable
* Triangle Clipping (Screen Space)
* Rastarizer
* Barycentric Interpolator
* Depth & Alpha Blending

<br>

# Examples

The examples for this has been made similar to https://learnopengl.com/

### Hello Triangle [source](./Sandbox/Source/01_HelloTriangle.cpp)

<img width=500 src="./Images/01_hellotriangle.png" alt="Hello Triangle" />

### Textues [source](./Sandbox/Source/02_Textures.cpp)

<img width=500 src="./Images/02_textures.png" alt="Textures" />

### Transforms [source](./Sandbox/Source/03_Transforms.cpp)

<img width=500 src="./Images/03_transforms.png" alt="Transforms" />

### Going 3D [source](./Sandbox/Source/04_Going3D.cpp)

<img width=500 src="./Images/04_going3d.png" alt="Going 3D" />

### Model Loading [source](./Sandbox/Source/05_ModelLoading.cpp)

<img width=500 src="./Images/05_modelloading.png" alt="Model Loading" />

### PBR [source](./Sandbox/Source/06_PBR.cpp)

<img width=500 src="./Images/06_pbr.png" alt="PBR" />

### ImGui Integration [source](./Sandbox/Source/07_ImGui.cpp)

<img width=500 src="./Images/07_imgui.png" alt="PBR" />


More comming soon...


