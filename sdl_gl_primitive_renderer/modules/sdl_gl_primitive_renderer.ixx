// sdl_gl_primitive_renderer.ixx
// OpenGL backend of PrimitiveRenderer, fully self-contained with SDL + GL.
//

module;
#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include "glad/gl.h"

#include <cassert>
#include <cstdint>
#include <expected>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <vector>
export module sdl_gl_primitive_renderer;

import primitive_renderer; // PrimitiveRenderer / RenderProcedureHandle / BufferHandle ...
import data_type; // checkable / GpuError / ImageData 等

namespace sopho
{
    struct GLRenderProcedure
    {
        GLuint program{};
    };

    struct GLBuffer
    {
        GLuint id{};
        GLenum target{};
        GLsizeiptr size{};
    };

    struct GLTexture
    {
        GLuint id{};
        GLenum target{GL_TEXTURE_2D};
        int width{};
        int height{};
    };

    struct GLUniformBuffer
    {
        GLuint id{};
        GLsizeiptr size{};
    };

    export class SDLGLPrimitiveRenderer : public PrimitiveRenderer
    {
        SDL_Window* m_window{nullptr};
        SDL_GLContext m_gl_context{nullptr};
        int m_width{1280};
        int m_height{720};

        GLuint m_vao{};

        std::map<RenderProcedureHandle, GLRenderProcedure> m_render_procedures{};
        std::map<TextureHandle, GLTexture> m_textures{};
        std::map<BufferHandle, GLBuffer> m_buffers{};

        std::map<std::int32_t, GLUniformBuffer> m_vertex_uniform_buffers{};
        std::map<std::int32_t, GLUniformBuffer> m_fragment_uniform_buffers{};

    public:
        static checkable<PrimitiveRenderer*> create()
        {
            auto* renderer = new SDLGLPrimitiveRenderer{};
            if (!renderer->init_sdl_and_gl())
            {
                delete renderer;
                return std::unexpected(static_cast<GpuError>(0));
            }
            return renderer;
        }

        ~SDLGLPrimitiveRenderer() override
        {
            for (auto& [_, rp] : m_render_procedures)
            {
                if (rp.program)
                {
                    glDeleteProgram(rp.program);
                }
            }

            for (auto& [_, tex] : m_textures)
            {
                if (tex.id)
                {
                    glDeleteTextures(1, &tex.id);
                }
            }

            for (auto& [_, buf] : m_buffers)
            {
                if (buf.id)
                {
                    glDeleteBuffers(1, &buf.id);
                }
            }

            for (auto& [_, ub] : m_vertex_uniform_buffers)
            {
                if (ub.id)
                {
                    glDeleteBuffers(1, &ub.id);
                }
            }

            for (auto& [_, ub] : m_fragment_uniform_buffers)
            {
                if (ub.id)
                {
                    glDeleteBuffers(1, &ub.id);
                }
            }

            if (m_vao)
            {
                glDeleteVertexArrays(1, &m_vao);
            }

            if (m_gl_context)
            {
                SDL_GL_DestroyContext(m_gl_context);
                m_gl_context = nullptr;
            }
            if (m_window)
            {
                SDL_DestroyWindow(m_window);
                m_window = nullptr;
            }
        }

        void begin_frame() override
        {
            SDL_GL_MakeCurrent(m_window, m_gl_context);

            int w = 0, h = 0;
            SDL_GetWindowSizeInPixels(m_window, &w, &h);
            if (w != 0 && h != 0)
            {
                m_width = w;
                m_height = h;
            }
            glViewport(0, 0, m_width, m_height);

            glBindVertexArray(m_vao);
        }

        void end_frame() override
        {
            glFlush();
            SDL_GL_SwapWindow(m_window);
        }

        void begin_render_pass(const RenderPassDescriptor& render_pass_descriptor) override
        {
            GLbitfield clear_mask = 0;

            if (render_pass_descriptor.clear)
            {
                clear_mask |= GL_COLOR_BUFFER_BIT;
                glClearColor(135.0f / 255.0f, 135.0f / 255.0f, 135.0f / 255.0f, 1.0f);
            }

            if (render_pass_descriptor.depth)
            {
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
                clear_mask |= GL_DEPTH_BUFFER_BIT;
            }
            else
            {
                glDisable(GL_DEPTH_TEST);
            }

            if (clear_mask != 0)
            {
                glClear(clear_mask);
            }
        }

        void end_render_pass() override {}

        checkable<RenderProcedureHandle>
        create_render_procedure(const RenderProcedureDescriptor& render_procedure_descriptor) override
        {
            GLuint vs = compile_shader(GL_VERTEX_SHADER, render_procedure_descriptor.vert_shader.c_str());
            if (!vs)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to compile vertex shader");
                return std::unexpected(static_cast<GpuError>(0));
            }

            GLuint fs = compile_shader(GL_FRAGMENT_SHADER, render_procedure_descriptor.frag_shader.c_str());
            if (!fs)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to compile fragment shader");
                glDeleteShader(vs);
                return std::unexpected(static_cast<GpuError>(0));
            }

            GLuint program = glCreateProgram();
            glAttachShader(program, vs);
            glAttachShader(program, fs);
            glLinkProgram(program);

            glDeleteShader(vs);
            glDeleteShader(fs);

            GLint linked = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &linked);
            if (!linked)
            {
                GLint log_len = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_len);
                std::string log(static_cast<std::size_t>(log_len), '\0');
                glGetProgramInfoLog(program, log_len, nullptr, log.data());
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to link program: %s", log.c_str());
                glDeleteProgram(program);
                return std::unexpected(static_cast<GpuError>(0));
            }

            RenderProcedureHandle handle{};
            if (!m_render_procedures.empty())
            {
                handle = static_cast<RenderProcedureHandle>(
                    static_cast<std::uint32_t>(m_render_procedures.rbegin()->first) + 1);
            }

            m_render_procedures.emplace(handle, GLRenderProcedure{program});
            return handle;
        }

        void bind_render_procedure(const RenderProcedureHandle& render_procedure_handle) override
        {
            auto it = m_render_procedures.find(render_procedure_handle);
            if (it == m_render_procedures.end())
            {
                return;
            }
            glUseProgram(it->second.program);
        }

        checkable<TextureHandle> create_texture(const ImageData& image_data) override
        {
            GLuint tex = 0;
            glGenTextures(1, &tex);
            if (!tex)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create GL texture");
                return std::unexpected(static_cast<GpuError>(0));
            }

            GLenum format = GL_RGBA;
            switch (image_data.channels)
            {
            case 1:
                format = GL_RED;
                break;
            case 2:
                format = GL_RG;
                break;
            case 3:
                format = GL_RGB;
                break;
            case 4:
            default:
                format = GL_RGBA;
                break;
            }

            glBindTexture(GL_TEXTURE_2D, tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(format), image_data.width, image_data.height, 0, format,
                         GL_UNSIGNED_BYTE,
                         image_data.pixels.empty() ? nullptr : reinterpret_cast<const void*>(image_data.pixels.data()));
            glBindTexture(GL_TEXTURE_2D, 0);

            TextureHandle handle{};
            if (!m_textures.empty())
            {
                handle = static_cast<TextureHandle>(static_cast<std::uint32_t>(m_textures.rbegin()->first) + 1);
            }
            m_textures.emplace(handle, GLTexture{tex, GL_TEXTURE_2D, image_data.width, image_data.height});
            return handle;
        }

        void bind_texture(const TextureHandle& texture_handle) override
        {
            auto it = m_textures.find(texture_handle);
            if (it == m_textures.end())
            {
                return;
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(it->second.target, it->second.id);
        }

        checkable<BufferHandle> create_buffer(const BufferDescriptor& buffer_descriptor) override
        {
            GLenum target{};
            switch (buffer_descriptor.buffer_usage)
            {
            case BufferUsage::VERTEX:
                target = GL_ARRAY_BUFFER;
                break;
            case BufferUsage::INDEX:
                target = GL_ELEMENT_ARRAY_BUFFER;
                break;
            default:
                assert(!"Invalid buffer usage");
                target = GL_ARRAY_BUFFER;
                break;
            }

            GLuint id = 0;
            glGenBuffers(1, &id);
            if (!id)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create GL buffer");
                return std::unexpected(static_cast<GpuError>(0));
            }

            glBindBuffer(target, id);
            glBufferData(target, static_cast<GLsizeiptr>(buffer_descriptor.data.size()),
                         buffer_descriptor.data.empty() ? nullptr
                                                        : reinterpret_cast<const void*>(buffer_descriptor.data.data()),
                         GL_STATIC_DRAW);
            glBindBuffer(target, 0);

            BufferHandle handle{};
            if (!m_buffers.empty())
            {
                handle = static_cast<BufferHandle>(static_cast<std::uint32_t>(m_buffers.rbegin()->first) + 1);
            }
            m_buffers.emplace(handle, GLBuffer{id, target, static_cast<GLsizeiptr>(buffer_descriptor.data.size())});
            return handle;
        }

        void bind_vertex_buffer(const BufferHandle& buffer_handle) override
        {
            auto it = m_buffers.find(buffer_handle);
            if (it == m_buffers.end())
            {
                return;
            }
            const auto& buf = it->second;
            assert(buf.target == GL_ARRAY_BUFFER);

            glBindVertexArray(m_vao);
            glBindBuffer(GL_ARRAY_BUFFER, buf.id);

            constexpr GLsizei stride = sizeof(float) * (3 + 3 + 2);
            GLsizei offset = 0;

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void*>(static_cast<std::intptr_t>(offset)));
            offset += sizeof(float) * 3;

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void*>(static_cast<std::intptr_t>(offset)));
            offset += sizeof(float) * 3;

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                                  reinterpret_cast<const void*>(static_cast<std::intptr_t>(offset)));
        }

        void bind_index_buffer(const BufferHandle& buffer_handle) override
        {
            auto it = m_buffers.find(buffer_handle);
            if (it == m_buffers.end())
            {
                return;
            }
            const auto& buf = it->second;
            assert(buf.target == GL_ELEMENT_ARRAY_BUFFER);

            glBindVertexArray(m_vao);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf.id);
        }

        void push_vertex_uniform(const UniformDescriptor& uniform_descriptor) override
        {
            push_uniform_impl(uniform_descriptor, m_vertex_uniform_buffers);
        }

        void push_fragment_uniform(const UniformDescriptor& uniform_descriptor) override
        {
            push_uniform_impl(uniform_descriptor, m_fragment_uniform_buffers);
        }

        void draw_index(std::int32_t num_indices) override
        {
            glBindVertexArray(m_vao);
            glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);
        }

    private:
        SDLGLPrimitiveRenderer() = default;

        bool init_sdl_and_gl()
        {
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init failed: %s", SDL_GetError());
                return false;
            }

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

            m_window = SDL_CreateWindow("SDL GL PrimitiveRenderer", m_width, m_height,
                                        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
            if (!m_window)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_CreateWindow failed: %s", SDL_GetError());
                return false;
            }

            m_gl_context = SDL_GL_CreateContext(m_window);
            if (!m_gl_context)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_GL_CreateContext failed: %s", SDL_GetError());
                return false;
            }

            if (SDL_GL_MakeCurrent(m_window, m_gl_context) < 0)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_GL_MakeCurrent failed: %s", SDL_GetError());
                return false;
            }

            if (SDL_GL_SetSwapInterval(1) < 0)
            {
                SDL_LogWarn(SDL_LOG_CATEGORY_VIDEO, "Failed to enable vsync: %s", SDL_GetError());
            }

            if (!init_gl_loader())
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to init GL loader");
                return false;
            }

            glGenVertexArrays(1, &m_vao);
            if (!m_vao)
            {
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to create VAO");
                return false;
            }
            glBindVertexArray(m_vao);

            // glEnable(GL_CULL_FACE);
            // glCullFace(GL_BACK);
            // glFrontFace(GL_CCW);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);

            SDL_Log("OpenGL Version: %s", glGetString(GL_VERSION));
            SDL_Log("OpenGL Renderer: %s", glGetString(GL_RENDERER));

            return true;
        }

        bool init_gl_loader()
        {
            if (!gladLoadGL(SDL_GL_GetProcAddress))
                return false;
            return true;
        }

        static GLuint compile_shader(GLenum type, const char* source)
        {
            GLuint shader = glCreateShader(type);
            if (!shader)
            {
                return 0;
            }

            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);

            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (!compiled)
            {
                GLint log_len = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
                std::string log(static_cast<std::size_t>(log_len), '\0');
                glGetShaderInfoLog(shader, log_len, nullptr, log.data());
                SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to compile shader: %s", log.c_str());
                glDeleteShader(shader);
                return 0;
            }

            return shader;
        }

        void push_uniform_impl(const UniformDescriptor& desc, std::map<std::int32_t, GLUniformBuffer>& buffers)
        {
            if (desc.slot_index < 0)
            {
                return;
            }

            auto it = buffers.find(desc.slot_index);
            if (it == buffers.end())
            {
                GLUniformBuffer ub{};
                glGenBuffers(1, &ub.id);
                ub.size = static_cast<GLsizeiptr>(desc.data.size());

                glBindBuffer(GL_UNIFORM_BUFFER, ub.id);
                glBufferData(GL_UNIFORM_BUFFER, ub.size,
                             desc.data.data() ? reinterpret_cast<const void*>(desc.data.data()) : nullptr,
                             GL_DYNAMIC_DRAW);
                glBindBuffer(GL_UNIFORM_BUFFER, 0);

                glBindBufferBase(GL_UNIFORM_BUFFER, static_cast<GLuint>(desc.slot_index), ub.id);

                buffers.emplace(desc.slot_index, ub);
            }
            else
            {
                auto& ub = it->second;
                glBindBuffer(GL_UNIFORM_BUFFER, ub.id);

                GLsizeiptr new_size = static_cast<GLsizeiptr>(desc.data.size());
                if (new_size > ub.size)
                {
                    ub.size = new_size;
                    glBufferData(GL_UNIFORM_BUFFER, ub.size,
                                 desc.data.data() ? reinterpret_cast<const void*>(desc.data.data()) : nullptr,
                                 GL_DYNAMIC_DRAW);
                }
                else
                {
                    glBufferSubData(GL_UNIFORM_BUFFER, 0, new_size,
                                    desc.data.data() ? reinterpret_cast<const void*>(desc.data.data()) : nullptr);
                }
                glBindBuffer(GL_UNIFORM_BUFFER, 0);
            }
        }
    };

} // namespace sopho
