#ifndef MUSH_EXTRA_GFX_SHADER
#define MUSH_EXTRA_GFX_SHADER

#ifdef MUSH_MAKE_IMPLEMENTATIONS
    #define MUSH_IMPLEMENT_SHADER
#endif

#include <cstdint>

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../../string.hpp"
#include "../../metastring.hpp"

#include "shader_common.hpp"
#include "shadergen.hpp"
#include "vertex.hpp"

namespace mush::extra::opengl
{

    // Type traits
    template <typename T>
    struct matrix_type
    {
       static constexpr bool value = false;
       static constexpr int order = -1;
    };

    template <>
    struct matrix_type<glm::mat2>
    {
       static constexpr bool value = true;
       static constexpr int order = 2;
    };

    template <>
    struct matrix_type<glm::mat3>
    {
       static constexpr bool value = true;
       static constexpr int order = 3;
    };

    template <>
    struct matrix_type<glm::mat4>
    {
       static constexpr bool value = true;
       static constexpr int order = 4;
    };

    template <typename T>
    concept bool Matrix = matrix_type<T>::value;

    template <uint32_t Dummy>
    struct Shader_Common
    {
        protected:
            static GLint current_program;
    };

    template <uint32_t Dummy>
    GLint Shader_Common<Dummy>::current_program;

    template <AnyVertexType VT, ShaderType ST = VERTEX_FRAGMENT>
    class Shader : Shader_Common<0>
    {
        private:
            GLint program;

        protected:
            struct UniformAssignment
            {
                union
                {
                    struct { GLint sx,sy,sz,sw; };
                    struct { GLuint ux,uy,uz,uw; };
                    struct { GLfloat x,y,z,w; };
                };

                int order;
                int type;

                // Vectors
                // 4D
                UniformAssignment(GLint x, GLint y, GLint z, GLint w) :
                sx(x), sy(y), sz(z), sw(w), order(4), type(0) {}
                UniformAssignment(GLuint x, GLuint y, GLuint z, GLuint w) :
                ux(x), uy(y), uz(z), uw(w), order(4), type(1) {}
                UniformAssignment(GLfloat x, GLfloat y, GLfloat z, GLfloat w) :
                x(x), y(y), z(z), w(w), order(4), type(2) {}

                // 3D
                UniformAssignment(GLint x, GLint y, GLint z) : sx(x), sy(y), sz(z), order(3), type(0) {}
                UniformAssignment(GLuint x, GLuint y, GLuint z) : ux(x), uy(y), uz(z), order(3), type(1){}
                UniformAssignment(GLfloat x, GLfloat y, GLfloat z) : x(x), y(y), z(z), order(3), type(2) {}

                // 2D
                UniformAssignment(GLint x, GLint y) : sx(x), sy(y), order(2), type(0) {}
                UniformAssignment(GLuint x, GLuint y) : ux(x), uy(y), order(2), type(1) {}
                UniformAssignment(GLfloat x, GLfloat y) : x(x), y(y), order(2), type(2) {}

                // 1D
                UniformAssignment(GLint x) : sx(x), order(1), type(0) {}
                UniformAssignment(GLuint x) : ux(x), order(1), type(1) {}
                UniformAssignment(GLfloat x) : x(x), order(1), type(2) {}
            };

            class UniformProxy
            {
                GLint location;
                GLint program;
                
                mush::String name;

                public:
                    UniformProxy(const mush::String& name, GLint program) : program(program), name(name)
                    {
                        while (glGetError() != GL_NO_ERROR);

                        location = glGetUniformLocation(program, name.std_str().c_str());

                        if (location == -1)
                            std::cout << "shader error: no uniform '" << name << "'\n";
                    }

                    void operator=(UniformAssignment val)
                    {
                        if (location == -1)
                            return;

                        GLint oldprog = current_program;
                        if (program != current_program)
                            glUseProgram(program);

                        while (glGetError() != GL_NO_ERROR)
                        if (val.type == 0) // GLint
                        {
                            if (val.order == 1) glUniform1i(location, val.sx);
                            if (val.order == 2) glUniform2i(location, val.sx, val.sy);
                            if (val.order == 3) glUniform3i(location, val.sx, val.sy, val.sz);
                            if (val.order == 4) glUniform4i(location, val.sx, val.sy, val.sz, val.sw);
                        } else if (val.type == 1) { // GLuint
                            if (val.order == 1) glUniform1ui(location, val.ux);
                            if (val.order == 2) glUniform2ui(location, val.ux, val.uy);
                            if (val.order == 3) glUniform3ui(location, val.ux, val.uy, val.uz);
                            if (val.order == 4) glUniform4ui(location, val.ux, val.uy, val.uz, val.uw);
                        } else if (val.type == 2) { // GLfloat
                            if (val.order == 1) glUniform1f(location, val.x);
                            if (val.order == 2) glUniform2f(location, val.x, val.y);
                            if (val.order == 3) glUniform3f(location, val.x, val.y, val.z);
                            if (val.order == 4) glUniform4f(location, val.x, val.y, val.z, val.w);
                        }

                        if (glGetError() != GL_NO_ERROR)
                        {
                            std::cout << "Error assigning value to shader uniform '" << name << "'. (type mismatch?)\n";
                        }

                        if (oldprog != current_program)
                            glUseProgram(oldprog);
                    }

                    template <Matrix T>
                    void operator=(const T& mat)
                    {
                        if (location == -1)
                            return;

                        GLint oldprog = current_program;
                        if (program != current_program)
                            glUseProgram(program);

                        if constexpr (matrix_type<T>::order == 2)
                            glUniformMatrix2fv(location, 1, GL_FALSE, &mat[0][0]);
                        if constexpr (matrix_type<T>::order == 3)
                            glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
                        if constexpr (matrix_type<T>::order == 4)
                            glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);

                        if (oldprog != current_program)
                            glUseProgram(oldprog);
                    }
            };

        public:
            typedef VT vertex_type;

            bool ok() const { return program != 0; }

            // do not allow copies
            Shader(const Shader& other) = delete;
            Shader(Shader& other) = delete;

            /*
            // but allow moving
            Shader operator=(Shader&& other)
            {
                return Shader(std::forward<Shader>(other));
            }
            */

            Shader()
            {
                program = 0;
            }

            Shader(Shader&& other)
            {
                program = other.program;
                other.program = 0;
            }

            void load_glsl(const char* vertex_source, const char* fragment_source)
            {
                int vertex_shader, fragment_shader;
                int status;

                vertex_shader = glCreateShader(GL_VERTEX_SHADER);
                glShaderSource(vertex_shader, 1, &vertex_source, 0);
                glCompileShader(vertex_shader);
                glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);

                if (!status)
                {
                    std::cout << "Vertex shader compile error" << "\n";
                    std::cout << vertex_source << "\n";
                    GLint maxLength = 0;
                    glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

                    // The maxLength includes the NULL character
                    std::vector<char> errorLog(maxLength);
                    glGetShaderInfoLog(vertex_shader, maxLength, &maxLength, &errorLog[0]);

                    std::string msg(errorLog.begin(), errorLog.end());
                    std::cout << msg << "\n";

                    program = 0;
                    return;
                }

                fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
                glShaderSource(fragment_shader, 1, &fragment_source, 0);
                glCompileShader(fragment_shader);
                glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);

                if (!status)
                {
                    std::cout << "Fragment shader compile error" << "\n";
                    
                    GLint maxLength = 0;
                    glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &maxLength);

                    // The maxLength includes the NULL character
                    std::vector<char> errorLog(maxLength);
                    glGetShaderInfoLog(fragment_shader, maxLength, &maxLength, &errorLog[0]);

                    std::string msg(errorLog.begin(), errorLog.end());
                    std::cout << msg << "\n";

                    program = 0;
                    return;;
                }

                program = glCreateProgram();
                glAttachShader(program, vertex_shader);
                glAttachShader(program, fragment_shader);
                glLinkProgram(program);
                glGetProgramiv(program, GL_LINK_STATUS, (int*)&status);

                if (!status)
                {
                    std::cout << "  shader linking error!\n";
                    glDeleteProgram(program);
                    program = 0;
                    return;
                }
                return;
            }

            void load_spirv();

            inline UniformProxy operator[](const mush::String& name)
            {
                assert(program != 0);
                return UniformProxy(name, program);
            }

            void use()
            {
                if (current_program == program)
                    return;

                if (program == 0)
                {
                    std::cout << "can't use incomplete shader\n";
                    std::abort();
                    return;
                }

                current_program = program;
                glUseProgram(program);
            }

           ~Shader()
            {
                if (program != 0)
                {
                    std::cout << "deleting program " << program << "\n"; 
                    glDeleteProgram(program);
                }
            }

            // make this the better generator
            template <ShaderType ShaderT, extra::opengl::AnyVertexType VertT = VT>
            void generate(Shader_Info<ShaderT, VertT> info)
            {
            }

            // the shader generation sucks, make better one that actually reads shader_info struct
            // and handles things like MRT.
            void generate_default()
            {
                static_assert(vertex_type::uv_count < 2 && "generate_default() doesn't support > 1 texture coordinates");

                using namespace mush::detail;

                auto vertex_source = make_string("")
                    .append(shadergen::generate_header<0>())
                    .append(shadergen::generate_pos_input<vertex_type::dim>())
                    .append(shadergen::generate_UV_inputs<VERTEX_SHADER, 0, vertex_type::uv_count>())
                    .append(shadergen::generate_ext_inputs<VERTEX_SHADER, vertex_type::uv_count+1, vertex_type::flags>())
                    .append(shadergen::generate_vs_outputs<0, vertex_type::uv_count, vertex_type::flags>())
                    .append(shadergen::generate_vs_main<vertex_type::dim, 0, vertex_type::uv_count, vertex_type::flags>());

                auto fragment_source = make_string("")
                    .append(shadergen::generate_header<1>())
                    .append(shadergen::generate_UV_inputs<FRAGMENT_SHADER, 0, vertex_type::uv_count>())
                    .append(shadergen::generate_ext_inputs<FRAGMENT_SHADER, vertex_type::uv_count+1, vertex_type::flags>())
                    .append("out vec4 outc;\n")
                    .append(shadergen::generate_fs_main<vertex_type::uv_count, vertex_type::flags>());
                
                //std::cout << vertex_source << "\n" << fragment_source << "\n";
                std::cout << "generating default shader for VertexType<" << vertex_type::dim << "," << vertex_type::uv_count << ",???>\n";
                load_glsl(vertex_source.c_str(), fragment_source.c_str());
            }
    };
    /*
    template <AnyVertexType VT, ShaderType ST>
    GLint Shader<VT,ST>::current_program;
    */

}

#endif
