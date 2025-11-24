// glsl_reflector.ixx
// Created by sophomore on 11/19/25.
//
module;
#include <iostream>
#include <string>
#include <vector>
#include "glslang/Include/intermediate.h"
#include "glslang/MachineIndependent/localintermediate.h"
#include "glslang/Public/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"
export module glsl_reflector;
export import data_type;

namespace sopho
{

    class GlslangProcessGuard
    {
    public:
        GlslangProcessGuard() { glslang::InitializeProcess(); }
        ~GlslangProcessGuard() { glslang::FinalizeProcess(); }
    };

    auto to_basic_type(glslang::TBasicType gl_basic_type)
    {
        switch (gl_basic_type)
        {
        case glslang::EbtFloat:
            return BasicType::FLOAT;
        default:
            return BasicType::NONE;
        }
    }

    export auto reflect_vertex(const std::string& vertex_source)
    {
        GlslangProcessGuard glslang_initializer{};
        VertexReflection result{};
        glslang::TShader shader(EShLangVertex);
        std::vector sources{vertex_source.data()};
        shader.setStrings(sources.data(), sources.size());

        int clientInputSemanticsVersion = 100; // not used for desktop GL
        glslang::EShTargetClientVersion clientVersion = glslang::EShTargetOpenGL_450;
        glslang::EShTargetLanguageVersion targetVersion = glslang::EShTargetSpv_1_0;

        shader.setEnvInput(glslang::EShSourceGlsl, EShLangVertex, glslang::EShClientOpenGL,
                           clientInputSemanticsVersion);
        shader.setEnvClient(glslang::EShClientOpenGL, clientVersion);
        shader.setEnvTarget(glslang::EShTargetSpv, targetVersion);

        EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);

        if (!shader.parse(GetDefaultResources(), 450, ENoProfile, false, false, messages))
        {
            std::cerr << "Parse failed:\n" << shader.getInfoLog() << std::endl;
            return result;
        }

        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(messages))
        {
            std::cerr << "Link failed:\n" << program.getInfoLog() << std::endl;
            return result;
        }
        program.buildReflection();
        auto count = program.getNumPipeInputs();
        for (auto i = 0; i < count; i++)
        {
            const auto& var = program.getPipeInput(i);

            const auto& type = var.getType();
            const auto& q = type->getQualifier();

            std::string name = var.name.c_str();
            auto vector = type->getVectorSize();
            std::cout << name << vector << std::endl;
            result.inputs.emplace_back(VertexInfo{.location = var.layoutLocation(),
                                                  .name = var.name,
                                                  .basic_type = to_basic_type(type->getBasicType()),
                                                  .vector_size = type->getVectorSize()});
        }
        return result;
    }
} // namespace sopho
