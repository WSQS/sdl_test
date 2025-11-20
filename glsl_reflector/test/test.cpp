// test.cpp
// Created by wsqsy on 11/19/2025.
//
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include "glslang/Include/intermediate.h"
#include "glslang/MachineIndependent/localintermediate.h"
#include "glslang/Public/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"

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
