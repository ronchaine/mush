#ifndef MUSH_EXTRA_GFX_SHADER
#define MUSH_EXTRA_GFX_SHADER

#ifndef MUSH_MAKE_IMPLEMENTATIONS
    #define MUSH_IMPLEMENT_SHADER
#endif

#include <cstdint>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>

namespace mush::extra::opengl
{
    class Shader
    {
        private:
            static GLuint current_program;
            GLuint program;

        protected:

        public:
            Shader load_glsl(const char* vertex_source, const char* fragment_source);
            Shader load_spirv();

            void use();

           ~Shader();
    };

    #ifdef MUSH_IMPLEMENT_SHADER
    Shader::~Shader()
    {
        if (program != 0)
            glDeleteProgram(program);
    }

    Shader Shader::load_glsl(const char* vertex_source, const char* fragment_source)
    {
        Shader rval;

        int vertex_shader, fragment_shader, program;
        int status;

        std::cout << "Compiling shaders...\n";

        vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        std::cout << "  created shader " << vertex_shader << "\n";
        glShaderSource(vertex_shader, 1, &vertex_source, 0);
        std::cout << "  source code read\n";
        glCompileShader(vertex_shader);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);

        if (!status)
        {
            std::cout << "Vertex shader compile error" << "\n";
            GLint maxLength = 0;
            glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<char> errorLog(maxLength);
            glGetShaderInfoLog(vertex_shader, maxLength, &maxLength, &errorLog[0]);

            std::string msg(errorLog.begin(), errorLog.end());
            std::cout << msg << "\n";

            rval.program = 0;
            return rval;
        }

        std::cout << "  vertex shader compiled\n";

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

            rval.program = 0;
            return rval;
        }

        std::cout << "  fragment shader compiled\n";

        program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, (int*)&status);

        if (!status)
        {
            glDeleteProgram(program);
            rval.program = 0;
            return rval;
        }

        rval.program = program;
        return rval;
    }
    #endif
}

#endif
