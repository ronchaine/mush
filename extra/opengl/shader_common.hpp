#ifndef MUSH_EXTRA_GFX_SHADER_COMMON
#define MUSH_EXTRA_GFX_SHADER_COMMON

#include <cstdint>

#include "vertex.hpp"

namespace mush
{
    using ShaderGenFlags = uint32_t;
    using ShaderType = uint32_t;

    constexpr static ShaderType VERTEX_FRAGMENT = 0x00;
    constexpr static ShaderType COMPUTE = 0x01;

    constexpr static ShaderType VERTEX_SHADER = 0xa0;
    constexpr static ShaderType FRAGMENT_SHADER = 0xa1;

    template <ShaderType ShaderT, extra::opengl::AnyVertexType VertexT>
    struct Shader_Info
    {
        typedef VertexT vertex_type;
    };
}

#endif
