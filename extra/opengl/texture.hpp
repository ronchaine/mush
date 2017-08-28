#ifndef MUSH_EXTRA_GFX_TEXTURE
#define MUSH_EXTRA_GFX_TEXTURE

#include <cstdint>
#include <cassert>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace mush::extra::opengl
{
    class Texture
    {
        private:
            GLuint texture;

            uint32_t width;
            uint32_t height;
            
        public:
            Texture() : texture(0), width(0), height(0)
            {
                glGenTextures(1, &texture);
            }

            Texture(uint32_t width, uint32_t height, uint32_t channels)
            {
                glGenTextures(1, &texture);
                init(width, height, channels);
            }

           ~Texture()
            {
                if (texture == 0)
                    return;

                glDeleteTextures(1, &texture);
            }

            const GLuint id()
            {
               return texture;
            }

            void init(uint32_t w, uint32_t h, uint32_t channels)
            {
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                glBindTexture(GL_TEXTURE_2D, texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
             
                glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);

                width = w;
                height = h;
            }
            
            void update(const void* image, uint32_t xoff, uint32_t yoff, uint32_t w, uint32_t h)
            {
                glBindTexture(GL_TEXTURE_2D, texture);
                glTexSubImage2D(GL_TEXTURE_2D, 0, xoff, yoff, w, h, GL_RGBA, GL_UNSIGNED_BYTE, image);
            }

            void create(const void* image, uint32_t w, uint32_t h, uint32_t channels)
            {
                init(w, h, channels);
                update(image, 0, 0, w, h);
            }

            void grow(uint32_t w, uint32_t h)
            {
                assert(w >= width && h >= height);

                GLuint new_texture;
                glGenTextures(1, &new_texture);
                glBindTexture(GL_TEXTURE_2D, new_texture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);

                glCopyImageSubData(texture, GL_TEXTURE_2D, 0, 0, 0, 0,
                                   new_texture,  GL_TEXTURE_2D, 0, 0, 0, 0,
                                   width, height, 1);

                glDeleteTextures(1, &texture);
                texture = new_texture;
            }

            void bind(GLint texture_unit)
            {
                GLint units;
                glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &units);

                if (texture_unit > units)
                   return;

                glActiveTexture(GL_TEXTURE0 + texture_unit);
                glBindTexture(GL_TEXTURE_2D, texture);
            }
   };
}

#endif
