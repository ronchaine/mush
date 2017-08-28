#ifndef MUSH_EXTRA_GFX_SHADERGEN
#define MUSH_EXTRA_GFX_SHADERGEN

#include <cstdint>

#include "shader_common.hpp"

#include "../../metastring.hpp"

namespace mush::detail::shadergen
{
        using ShaderType = uint32_t;

        template <int Version>
        constexpr auto generate_header()
        {
            return mush::make_string(
                    "#version 330\n"
                    "#extension GL_ARB_explicit_attrib_location : require\n"
                   );
        }

        template <>
        constexpr auto generate_header<1>()
        {
            return mush::make_string(
                    "#version 330\n"
                    "precision highp float;\n"
                   );
        }

        template <ShaderType Type, int N, int Last>
        constexpr auto generate_UV_inputs()
        {
            if constexpr (Type == VERTEX_SHADER)
            {
                if constexpr (N == Last)
                    return make_string("");
                else
                {
                    const mush::metastring s0 = make_string("layout(location = ").append(integer_to_metastring<N+1>()).append(")");
                    const mush::metastring s1 = s0.append(" in vec2 in_tex").append(integer_to_metastring<N>()).append(";\n");
                    return s1.append(generate_UV_inputs<Type, N+1, Last>());
                }
            } else if constexpr (Type == FRAGMENT_SHADER) {
                if constexpr (N == Last)
                    return make_string("");
                else
                {
                    const mush::metastring s0 = make_string("in vec2 ex_tex")
                        .append(integer_to_metastring<N>())
                        .append(";\n");
                    return s0.append(generate_UV_inputs<Type, N+1, Last>());
                }
            }
        }

        template <int N>
        constexpr auto generate_pos_input()
        {
            return make_string("layout(location = 0) in vec").append(integer_to_metastring<N>()).append(" in_pos;\n");
        }

        template <ShaderType Type, int N, extra::opengl::VertexTypeFlags Flags, bool NeedExCol = true>
        constexpr auto generate_ext_inputs()
        {
            if constexpr(Type == VERTEX_SHADER)
            {
                // add RGBA colours
                if constexpr(Flags & extra::opengl::VERTEX_RGBA_COLOUR)
                {
                    return make_string("layout(location = ").append(integer_to_metastring<N>()).append(") in vec4 in_col;\n").append(
                        generate_ext_inputs<Type, N+1, Flags & ~extra::opengl::VERTEX_RGBA_COLOUR>()
                    );
                }
                // add HSV shift
                else if constexpr(Flags & extra::opengl::VERTEX_HSV_SHIFT)
                {
                    return make_string("layout(location = ").append(integer_to_metastring<N>()).append(") in vec3 in_hsv;\n").append(
                        generate_ext_inputs<Type, N+1, Flags & ~extra::opengl::VERTEX_HSV_SHIFT>()
                    );
                }
                // end condition, all flags are handled
                else if constexpr(Flags == 0)
                {
                    return make_string("");
                }
                // if there were unhandled flags, fail
                else
                {
                    static_assert(dependent_false<decltype(Flags)>::value && "Unhandled Vertex Flags in shader generation");
                    return make_string("THIS SHOULD NEVER HAPPEN"); // return something for the type deduction
                }
            } else if constexpr(Type == FRAGMENT_SHADER)
            {
                // add RGBA colours
                if constexpr(Flags & extra::opengl::VERTEX_RGBA_COLOUR)
                {
                    return make_string("in vec4 ex_col;\n").append(
                        generate_ext_inputs<Type, N+1, Flags & ~extra::opengl::VERTEX_RGBA_COLOUR, false>()
                    );
                }
                // add HSV shift
                else if constexpr(Flags & extra::opengl::VERTEX_HSV_SHIFT)
                {
                    return make_string("in vec3 ex_hsv;\n").append(
                        generate_ext_inputs<Type, N+1, Flags & ~extra::opengl::VERTEX_HSV_SHIFT,NeedExCol>()
                    );
                }
                // if no colour, add ex_col = 1.0
                else if constexpr(NeedExCol)
                {
                    return make_string("const vec4 ex_col = vec4(1.0, 1.0, 1.0, 1.0);\n").append(
                        generate_ext_inputs<Type, N, Flags, false>()
                    );
                }
                // end condition, all flags are handled
                else if constexpr(Flags == 0)
                {
                    return make_string("");
                }
                // if there were unhandled flags, fail
                else
                {
                    static_assert(dependent_false<decltype(Flags)>::value && "Unhandled Vertex Flags in shader generation");
                    return make_string("THIS SHOULD NEVER HAPPEN"); // return something for the type deduction
                }
            }
        }

        template <int N, int Last, extra::opengl::VertexTypeFlags Flags>
        constexpr auto generate_vs_outputs()
        {
            if constexpr (N != Last)
            {
                return make_string("out vec2 ex_tex").append(integer_to_metastring<N>()).append(";\n")
                      .append(generate_vs_outputs<N+1, Last, Flags>());
            } else {
                if constexpr(Flags & extra::opengl::VERTEX_RGBA_COLOUR)
                {
                    return make_string("out vec4 ex_col;\n").append(
                        generate_vs_outputs<N, Last, Flags & ~extra::opengl::VERTEX_RGBA_COLOUR>()
                    );
                }
                // add HSV shift
                else if constexpr(Flags & extra::opengl::VERTEX_HSV_SHIFT)
                {
                    return make_string("out vec3 ex_hsv;\n").append(
                        generate_vs_outputs<N, Last, Flags & ~extra::opengl::VERTEX_HSV_SHIFT>()
                    );
                }
                else if constexpr(Flags == 0)
                {
                    return make_string("");
                }
                else
                {
                    static_assert(dependent_false<decltype(Flags)>::value && "Unhandled Vertex Flags in shader generation");
                    return make_string("THIS SHOULD NEVER HAPPEN"); // return something for the type deduction
                }
            }
        }

        template <int Pos, int N, int Last, extra::opengl::VertexTypeFlags Flags, bool Init = true>
        constexpr auto generate_vs_main()
        {
            if constexpr(Init)
            {
                return make_string("void main()\n{\n").append(generate_vs_main<Pos,N,Last,Flags,false>());
            }
            else if constexpr(N != Last)
            {
                return make_string("ex_tex").append(integer_to_metastring<N>())
                      .append(" = in_tex").append(integer_to_metastring<N>())
                      .append(";\n")
                      .append(generate_vs_main<Pos, N+1, Last, Flags,false>());
            }
            else if constexpr(Flags & extra::opengl::VERTEX_RGBA_COLOUR)
            {
                return make_string("ex_col = in_col;\n")
                      .append(generate_vs_main<Pos, N, Last, Flags & ~extra::opengl::VERTEX_RGBA_COLOUR, false>());
            }
            else if constexpr(Flags & extra::opengl::VERTEX_HSV_SHIFT)
            {
                return make_string("ex_hsv = in_hsv;\n")
                      .append(generate_vs_main<Pos, N, Last, Flags & ~extra::opengl::VERTEX_HSV_SHIFT, false>());
            }
            else if constexpr(Flags == 0)
            {
                if constexpr(Pos == 1) {
                    return make_string("gl_Position = vec4(in_pos, 0.0, 0.0, 1.0);\n}\n");
                } else if constexpr(Pos == 2) {
                    return make_string("gl_Position = vec4(in_pos, 0.0, 1.0);\n}\n");
                } else if constexpr(Pos == 3) {
                    return make_string("gl_Position = vec4(in_pos, 1.0);\n}\n");
                } else if constexpr(Pos == 4) {
                    return make_string("gl_Position = in_pos;\n}\n");
                }
            }
            else
            {
                static_assert(dependent_false<decltype(Flags)>::value && "code branch error in shadergen::generate_main()");
                return make_string("THIS SHOULD NEVER HAPPEN"); // return something for the type deduction
            }
        }

        template <int UV_Count,
                  extra::opengl::VertexTypeFlags Flags,
                  bool SamplersCreated = false,
                  bool HSVFunctionsCreated = false,
                  bool MainStarted = false>
        constexpr auto generate_fs_main()
        {
            if constexpr(SamplersCreated == false && UV_Count > 0)
            {
                return make_string("uniform sampler2D diffuse;\n")
                    .append(generate_fs_main<UV_Count, Flags, true>());
            }

            // create functions for HSV shifting
            else if constexpr(HSVFunctionsCreated == false && (Flags & extra::opengl::VERTEX_HSV_SHIFT))
            {
                return make_string(
                    "vec3 rgb2hsv(vec3 c)\n{\n"
                    "vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);\n"
                    "vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));\n"
                    "vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));\n"
                    "float d = q.x - min(q.w, q.y);\n"
                    "float e = 1.0e-10;\n"
                    "return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);\n}\n"

                    "vec3 hsv2rgb(vec3 c)\n{\n"
                    "vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);\n"
                    "vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);\n"
                    "return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);\n}\n"
                ).append(generate_fs_main<UV_Count,Flags,true,true>());
            } else if constexpr(MainStarted == false) {
                return make_string("void main()\n{\n").append(generate_fs_main<UV_Count,Flags,true,true,true>());
            } else if constexpr(UV_Count == 0) {
                if constexpr(Flags & extra::opengl::VERTEX_HSV_SHIFT)
                {
                    return make_string(
                        "vec3 hsv = rgb2hsv(ex_col.rgb);\n"
                        "hsv.r = hsv.r + ex_hsv.r;\n"
                        "hsv.g = hsv.g * ex_hsv.g;\n"
                        "hsv.b = hsv.b * ex_hsv.b;\n"
                        "outc = vec4(hsv2rgb(hsv), ex_col.a);\n"
                        "}\n"
                    );
                } else {
                    return make_string(
                        "outc = ex_col;\n"
                        "}\n"
                    );
                }
            } else {
                // We have a texture too
                if constexpr(Flags & extra::opengl::VERTEX_HSV_SHIFT)
                {
                    return make_string(
                        "vec4 diff = texture2D(diffuse,ex_tex0);\n"
                        "vec3 hsv = rgb2hsv(diff.rgb);\n"
                        "hsv.r = hsv.r + ex_hsv.r;\n"
                        "hsv.g = hsv.g * ex_hsv.g;\n"
                        "hsv.b = hsv.b * ex_hsv.b;\n"
                        "outc = ex_col * vec4(hsv2rgb(hsv), diff.a);\n"
                        "}\n"
                    );
                } else {
                    return make_string(
                        "outc = ex_col * texture2D(diffuse, ex_tex0);\n"
                        "}\n"
                    );
                }
            }
        }
        template <size_t N, typename... Rest>
        constexpr auto generate_sampler(metastring<N> name, Rest... rest)
        {
            if constexpr (sizeof...(Rest) == 0)
                return make_string("uniform sampler2D ").append(name).append(";\n");
            else
                return make_string("uniform sampler2D ").append(name).append(";\n").append(generate_sampler(rest...));
        }

        template <typename... Ts>
        constexpr auto generate_samplers(Ts... values)
        {
            return make_string("").append(generate_sampler(values...));
        }
}

#endif
