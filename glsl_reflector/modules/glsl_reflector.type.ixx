// glsl_reflector.type.ixx
// Created by wsqsy on 11/20/2025.
//
module;
#include <cstdint>
#include <string>
#include <vector>
export module glsl_reflector:type;

export namespace sopho
{
    enum class BasicType : std::uint8_t
    {
        NONE,
        FLOAT
    };

    struct VertexInfo
    {
        std::uint32_t location{};
        std::string name{};
        BasicType basic_type{};
        int vector_size{};
    };

    struct VertexReflection
    {
        std::vector<VertexInfo> inputs{};
    };
} // namespace sopho
