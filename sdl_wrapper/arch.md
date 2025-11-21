# SDL Wrapper Arch

```mermaid
graph TD

    subgraph RenderData
        BufferWrapper
    end

    Renderable --> RenderProcedural
    Renderable --> RenderData
    RenderProcedural --> GpuWrapper
    RenderData --> GpuWrapper

```
