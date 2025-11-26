// test.cpp
// Created by wsqsy on 11/19/2025.
//
#include <string>
#include <vector>

#include <gtest/gtest.h>

import glsl_reflector;

TEST(BasicTest, Basic) { EXPECT_EQ(1 + 1, 2); }

TEST(reflect_vertex, Basic)
{
    auto reflect_info = sopho::reflect_vertex(R"(
        #version 450
        layout(location = 0) in vec3 inPos;
        layout(location = 1) in vec3 inNormal;

        layout(location = 0) out vec3 outNormal;

        void main() {
            gl_Position = vec4(inPos, 1.0);
            outNormal = inNormal;
        }
    )");
    EXPECT_EQ(reflect_info.inputs.size(), 2);

    EXPECT_EQ(reflect_info.inputs[0].location, 0);
    EXPECT_EQ(reflect_info.inputs[0].name, "inPos");
    EXPECT_EQ(reflect_info.inputs[0].basic_type, sopho::BasicType::FLOAT);
    EXPECT_EQ(reflect_info.inputs[0].vector_size, 3);

    EXPECT_EQ(reflect_info.inputs[1].location, 1);
    EXPECT_EQ(reflect_info.inputs[1].name, "inNormal");
    EXPECT_EQ(reflect_info.inputs[1].basic_type, sopho::BasicType::FLOAT);
    EXPECT_EQ(reflect_info.inputs[1].vector_size, 3);
}


TEST(reflect_fragment, Basic)
{
    auto reflect_info = sopho::reflect_fragment(R"(
#version 460

layout (location = 0) in vec4 v_color;
layout (location = 0) out vec4 FragColor;

layout(set = 2, binding = 0) uniform sampler2D uTexture;

void main()
{
    FragColor = texture(uTexture, v_color.xy);
}
    )");
    EXPECT_EQ(reflect_info.sampler_count, 1);
}

TEST(reflect_fragment, Multi)
{
    auto reflect_info = sopho::reflect_fragment(R"(
#version 460

layout (location = 0) in vec4 v_color;
layout (location = 0) out vec4 FragColor;

layout(set = 2, binding = 0) uniform sampler2D uTexture;
layout(set = 2, binding = 1) uniform sampler2D uTexture1;

void main()
{
    FragColor = texture(uTexture, v_color.xy) + texture(uTexture1, v_color.xy);
}
    )");
    EXPECT_EQ(reflect_info.sampler_count, 2);
}
