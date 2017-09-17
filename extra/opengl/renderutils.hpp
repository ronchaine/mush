#ifndef MUSH_EXTRA_GFX_RENDERUTILS
#define MUSH_EXTRA_GFX_RENDERUTILS

#include <cstdint>

#include "vertex.hpp"
#include "shader.hpp"
#include "texture.hpp"

namespace mush::extra::opengl
{
    // first some voodoo to keep this stuff header-only and keep
    // ourselves consistent with render target
    template <uint32_t Dummy = 0>
    struct RenderTarget_Base
    {
        static GLuint           current_target;
    };
    template <uint32_t Dummy>
    GLuint RenderTarget_Base<Dummy>::current_target;

    template <uint32_t Display = 0>
    struct Screen_Info
    {
        static uint32_t width;
        static uint32_t height;

        static void update_size(uint32_t w, uint32_t h, bool set_view)
        {
            width = w;
            height = h;
            
            if (RenderTarget_Base<0>::current_target == 0 && set_view)
                glViewport(0,0,w,h);
        }
    };

    // instantiate as used
    template <uint32_t Display>
    uint32_t Screen_Info<Display>::width;
    template <uint32_t Display>
    uint32_t Screen_Info<Display>::height;

    inline void update_screen_size(uint32_t w, uint32_t h, bool set_viewport = true)
    {
        Screen_Info<0>::update_size(w, h, set_viewport);
    }
    
    inline uint32_t screen_width()
    {
        return Screen_Info<0>::width;
    }
    
    inline uint32_t screen_height()
    {
        return Screen_Info<0>::height;
    }

    template <uint32_t MRTLevel>
    class RenderTarget : RenderTarget_Base<0>
    {
        private:
            GLuint                  id;
            GLuint                  depth_buffer;

            const uint32_t          width, height;

            Texture                 colour_attachments[MRTLevel];

        public:
            RenderTarget(uint32_t width, uint32_t height, bool add_depth = false)
                : width(width), height(height)
            {
                glGenFramebuffers(1, &id);
                glBindFramebuffer(GL_FRAMEBUFFER, id);

                if (add_depth)
                {
                    glGenRenderbuffers(1, &depth_buffer);
                    glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
                } else {
                    depth_buffer = 0;
                }

                GLenum drawbuffers[MRTLevel];

                for (uint32_t i = 0; i < MRTLevel; ++i)
                {
                    colour_attachments[i].init(width, height, 4);
                    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, colour_attachments[i].id(), 0);
                    drawbuffers[i] = GL_COLOR_ATTACHMENT0 + i;
                }

                glDrawBuffers(1, drawbuffers);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                    assert(0 && "Failed to create render target");                
            }

           ~RenderTarget()
            {
                glDeleteFramebuffers(1, &id);
            }

            void use()
            {
                if (current_target == id)
                    return;

                current_target = id;

                glViewport(0.0, 0.0, width, height);
                glBindFramebuffer(GL_FRAMEBUFFER, id);
            }

            static void unset()
            {
                current_target = 0;
                glViewport(0.0, 0.0, Screen_Info<0>::width, Screen_Info<0>::height);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }
    };

    template <AnyVertexType T, ShaderType ST>
    void render_buffers(VertexBuffer<T>& buffer, Shader<T, ST>& shader)
    {
        assert(buffer.size() % sizeof(T) == 0);

        shader.use();

        GLuint vbo;
        glGenBuffers(1, &vbo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, buffer.size(), buffer.data(), GL_DYNAMIC_DRAW);

        uint32_t aa_count = 1 + T::uv_count + (T::has_rgba) + (T::has_hsv);

        for (uint32_t i = 0; i < aa_count; ++i)
            glEnableVertexAttribArray(i);

        glVertexAttribPointer(0, T::dim, GL_FLOAT, GL_FALSE, sizeof(T), (void*)0);

        uint32_t aapos = T::uv_count + 1;
        for (uint32_t pos = 1; pos < T::uv_count+1; ++pos)
            glVertexAttribPointer(pos, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(T), (void*)((pos-1+T::dim)*sizeof(float)));

        aapos = T::uv_count + 1;
        if constexpr(T::has_rgba)
        {
//            std::cout << (aapos + T::dim - 1) * sizeof(float) << "rgba:" << aapos << "\n";
            glVertexAttribPointer(aapos, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(T), (void*)((aapos + T::dim - 1)*sizeof(float)));
            aapos++;
        }
        if constexpr(T::has_hsv)
        {
            glVertexAttribPointer(aapos, 3, GL_FLOAT, GL_FALSE, sizeof(T), (void*)((aapos + T::dim - 1)*sizeof(float)));
            aapos++;
        }

        glDrawArrays(GL_TRIANGLES, 0, buffer.size() / sizeof(T));
        
        for (uint32_t i = 0; i < aa_count; ++i)
            glDisableVertexAttribArray(i);

        glDeleteBuffers(1, &vbo);
    }
}


#endif
