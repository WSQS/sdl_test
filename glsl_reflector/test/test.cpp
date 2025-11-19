#include <format>
#include <iostream>
#include <string>
#include <vector>

#include "glslang/Include/intermediate.h"
#include "glslang/MachineIndependent/localintermediate.h"
#include "glslang/Public/ResourceLimits.h"
#include "glslang/Public/ShaderLang.h"
static int s_index = 0;
class AstPrinter : public glslang::TIntermTraverser
{

    void printIndent() const
    {
        for (int i = 0; i < depth; ++i) // depth 是 TIntermTraverser 的成员
            std::cout << "  ";
    }
public:
    AstPrinter() :
        glslang::TIntermTraverser(
            /* preVisit = */ true,
            /* inVisit  = */ false,
            /* postVisit= */ true)
    {
    }

    virtual void visitSymbol(glslang::TIntermSymbol* symbol) override
    {
        printIndent();
        std::cout << __FUNCTION__ << '"' << symbol->getName() << '"' << symbol->getType().getQualifier().storage << std::endl;
    }
    virtual void visitConstantUnion(glslang::TIntermConstantUnion*) override
    {
        printIndent();
        std::cout << __FUNCTION__ << std::endl;
    }
    virtual bool visitBinary(glslang::TVisit, glslang::TIntermBinary* binary) override
    {
        printIndent();
        std::cout << __FUNCTION__ << std::endl;
        return true;
    }
    virtual bool visitUnary(glslang::TVisit, glslang::TIntermUnary*) override
    {
        printIndent();
        std::cout << __FUNCTION__ << std::endl;
        return true;
    }
    virtual bool visitSelection(glslang::TVisit, glslang::TIntermSelection*) override
    {
        printIndent();
        std::cout << __FUNCTION__ << std::endl;
        return true;
    }
    virtual bool visitAggregate(glslang::TVisit, glslang::TIntermAggregate* aggregate) override
    {
        printIndent();
        std::cout << std::format("{} \"{}\" {}", std::string(__FUNCTION__), aggregate->getName(),
                                 static_cast<int>(aggregate->getOp()))
                  << std::endl;
        return true;
    }
    virtual bool visitLoop(glslang::TVisit, glslang::TIntermLoop*) override
    {
        printIndent();
        std::cout << __FUNCTION__ << std::endl;
        return true;
    }
    virtual bool visitBranch(glslang::TVisit, glslang::TIntermBranch*) override
    {
        printIndent();
        std::cout << __FUNCTION__ << std::endl;
        return true;
    }
    virtual bool visitSwitch(glslang::TVisit, glslang::TIntermSwitch*) override
    {
        printIndent();
        std::cout << __FUNCTION__ << std::endl;
        return true;
    }
};


int main()
{
    // 1. 全局初始化
    glslang::InitializeProcess();

    const char* shaderSource = R"(
        #version 450
        layout(location = 0) in vec3 inPos;
        layout(location = 1) in vec3 inNormal;

        layout(location = 0) out vec3 outNormal;

        void main() {
            gl_Position = vec4(inPos, 1.0);
            outNormal = inNormal;
        }
    )";

    EShLanguage stage = EShLangVertex;
    glslang::TShader shader(stage);

    shader.setStrings(&shaderSource, 1);

    int clientInputSemanticsVersion = 100; // not used for desktop GL
    glslang::EShTargetClientVersion clientVersion = glslang::EShTargetOpenGL_450;
    glslang::EShTargetLanguageVersion targetVersion = glslang::EShTargetSpv_1_0;

    shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientOpenGL, clientInputSemanticsVersion);
    shader.setEnvClient(glslang::EShClientOpenGL, clientVersion);
    shader.setEnvTarget(glslang::EShTargetSpv, targetVersion);

    EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgSpvRules | EShMsgVulkanRules);

    if (!shader.parse(GetDefaultResources(), 450, ENoProfile, false, false, messages))
    {
        std::cerr << "Parse failed:\n" << shader.getInfoLog() << std::endl;
        glslang::FinalizeProcess();
        return 1;
    }

    // 2. link 成 program，得到 intermediate
    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(messages))
    {
        std::cerr << "Link failed:\n" << program.getInfoLog() << std::endl;
        glslang::FinalizeProcess();
        return 1;
    }

    glslang::TIntermediate* intermediate = program.getIntermediate(stage);
    if (!intermediate)
    {
        std::cerr << "No intermediate.\n";
        glslang::FinalizeProcess();
        return 1;
    }

    // 3. 拿 AST 根节点
    TIntermNode* root = intermediate->getTreeRoot();
    if (!root)
    {
        std::cerr << "No AST root.\n";
        glslang::FinalizeProcess();
        return 1;
    }

    // 在这里你可以开始自己遍历 root 来构建你自己的 AST/IR
    std::cout << "AST root acquired!" << std::endl;

    root->traverse(new AstPrinter());

    glslang::FinalizeProcess();
    return 0;
}
