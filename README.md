# SDL Test

This is a personal learning project following [Learn OpenGL](https://learnopengl.com/)

## Module

```mermaid
graph TD

    subgraph ThirdParty
        glslang
        shaderc
        SPIRVTools
        SDL3
        stb
        imgui
    end

    subgraph Modules
        data_type
        glsl_reflector
        sdl_wrapper
        logos
    end

    glslang --> shaderc
    SPIRVTools --> shaderc

    SDL3 --> imgui

    glslang --> glsl_reflector
    data_type --> glsl_reflector

    shaderc --> sdl_wrapper
    SDL3 --> sdl_wrapper
    glsl_reflector --> sdl_wrapper
    data_type --> sdl_wrapper
    
    imgui --> SDL_TEST
    sdl_wrapper --> SDL_TEST
    stb --> SDL_TEST
    logos --> SDL_TEST
```

## Track

- [ ] Getting started
    - [x] OpenGL
    - [x] Creating a window
    - [x] Hello Window
    - [x] Hello Triangle
    - [x] Shaders
    - [x] Textures
    - [x] Transformations
    - [ ] Coordinate Systems
    - [ ] Camera
- [ ] Lighting
- [ ] Model Loading
- [ ] Advanced OpenGL
- [ ] Advanced Lighting
- [ ] PBR
- [ ] In Practice
