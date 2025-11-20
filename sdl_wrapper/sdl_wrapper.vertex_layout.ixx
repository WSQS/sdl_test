// sdl_wrapper.vertex_attribute.ixx
// Created by wsqsy on 11/17/2025.
//
module;
#include <cstdint>
#include <vector>
#include "SDL3/SDL_gpu.h"
export module sdl_wrapper:vertex_layout;
import glsl_reflector;
namespace sopho
{

    export std::uint32_t get_size(SDL_GPUVertexElementFormat format)
    {
        switch (format)
        {
        default:
        case SDL_GPU_VERTEXELEMENTFORMAT_INVALID:
            return 0;
        case SDL_GPU_VERTEXELEMENTFORMAT_INT:
            return 4;
        case SDL_GPU_VERTEXELEMENTFORMAT_INT2:
            return 8;
        case SDL_GPU_VERTEXELEMENTFORMAT_INT3:
            return 12;
        case SDL_GPU_VERTEXELEMENTFORMAT_INT4:
            return 16;
        case SDL_GPU_VERTEXELEMENTFORMAT_UINT:
            return 4;
        case SDL_GPU_VERTEXELEMENTFORMAT_UINT2:
            return 8;
        case SDL_GPU_VERTEXELEMENTFORMAT_UINT3:
            return 12;
        case SDL_GPU_VERTEXELEMENTFORMAT_UINT4:
            return 16;
        case SDL_GPU_VERTEXELEMENTFORMAT_FLOAT:
            return 4;
        case SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2:
            return 8;
        case SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3:
            return 12;
        case SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4:
            return 16;
        case SDL_GPU_VERTEXELEMENTFORMAT_BYTE2:
            return 2;
        case SDL_GPU_VERTEXELEMENTFORMAT_BYTE4:
            return 4;
        case SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2:
            return 2;
        case SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4:
            return 4;
        case SDL_GPU_VERTEXELEMENTFORMAT_BYTE2_NORM:
            return 2;
        case SDL_GPU_VERTEXELEMENTFORMAT_BYTE4_NORM:
            return 4;
        case SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2_NORM:
            return 2;
        case SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM:
            return 4;
        case SDL_GPU_VERTEXELEMENTFORMAT_SHORT2:
            return 4;
        case SDL_GPU_VERTEXELEMENTFORMAT_SHORT4:
            return 8;
        case SDL_GPU_VERTEXELEMENTFORMAT_USHORT2:
            return 4;
        case SDL_GPU_VERTEXELEMENTFORMAT_USHORT4:
            return 8;
        case SDL_GPU_VERTEXELEMENTFORMAT_SHORT2_NORM:
            return 4;
        case SDL_GPU_VERTEXELEMENTFORMAT_SHORT4_NORM:
            return 8;
        case SDL_GPU_VERTEXELEMENTFORMAT_USHORT2_NORM:
            return 4;
        case SDL_GPU_VERTEXELEMENTFORMAT_USHORT4_NORM:
            return 8;
        case SDL_GPU_VERTEXELEMENTFORMAT_HALF2:
            return 4;
        case SDL_GPU_VERTEXELEMENTFORMAT_HALF4:
            return 8;
        }
    }

    export auto to_sdl_format(BasicType basic_type, int vector_size)
    {
        switch (basic_type)
        {
        case BasicType::FLOAT:
            {
                switch (vector_size)
                {
                case 3:
                    return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
                case 4:
                    return SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4;
                default:
                    return SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
                }
            }
        default:
            return SDL_GPU_VERTEXELEMENTFORMAT_INVALID;
        }
    }

    class VertexLayout
    {
        VertexReflection m_vertex_reflection{};
        std::vector<SDL_GPUVertexElementFormat> raw{};
        std::vector<SDL_GPUVertexAttribute> attributes{};
        uint32_t stride = 0;

    public:
        auto set_vertex_reflection(const VertexReflection& vertex_reflection)
        {
            m_vertex_reflection = vertex_reflection;
            stride = 0;
            attributes.clear();
            for (const auto& input : vertex_reflection.inputs)
            {
                SDL_GPUVertexAttribute vertex_attribute{};
                vertex_attribute.location = input.location;
                vertex_attribute.buffer_slot = 0;
                vertex_attribute.format = to_sdl_format(input.basic_type, input.vector_size);
                vertex_attribute.offset = stride;
                stride += get_size(vertex_attribute.format);
                attributes.push_back(vertex_attribute);
            }
        }
        const auto& get_vertex_reflection() { return m_vertex_reflection; }
        const auto& get_vertex_attributes() { return attributes; }
        auto get_stride() const { return stride; }
    };
} // namespace sopho
