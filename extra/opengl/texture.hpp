#ifndef MUSH_EXTRA_GFX_TEXTURE
#define MUSH_EXTRA_GFX_TEXTURE

#include <cstdint>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../../font.hpp"

namespace mush::extra::opengl
{
   class Texture
   {
      private:
         uint32_t texture;

      public:
         void create(const void* image, uint32_t w, uint32_t h, uint32_t channels)
         {
             glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

             glGenTextures(1, &texture);
             glBindTexture(GL_TEXTURE_2D, texture);
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
             glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
             
             glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, w, h);
             glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, image);
         }
   };

   Texture font_to_texture(const mush::Font_Base& font)
   {
   }
}

#endif
