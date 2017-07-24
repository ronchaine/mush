#ifndef MUSH_EXTRA_OPENGL_SHADER
#define MUSH_EXTRA_OPENGL_SHADER

#ifndef MUSH_MAKE_IMPLEMENTATIONS
#define MUSH_EXTRA_IMPLEMENT_OPENGL_SHADER
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>

namespace mush::extra::opengl
{
    using UniformType = int;
    constexpr UniformType UNIFORM_TYPE_GL_INT   = 0;
    constexpr UniformType UNIFORM_TYPE_GL_UINT  = 1;
    constexpr UniformType UNIFORM_TYPE_GL_FLOAT = 2;

    class Shader_Base
    {
        protected:
            // currently used program
            static GLuint program_in_use;

            GLint program;

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

                // 4D
                UniformAssignment(GLint x, GLint y, GLint z, GLint w) :
                    sx(x), sy(y), sz(z), sw(w), order(4), type(UNIFORM_TYPE_GL_INT) {}
                UniformAssignment(GLuint x, GLuint y, GLuint z, GLuint w) :
                    ux(x), uy(y), uz(z), uw(w), order(4), type(UNIFORM_TYPE_GL_UINT) {}
                UniformAssignment(GLfloat x, GLfloat y, GLfloat z, GLfloat w) :
                    x(x), y(y), z(z), w(w), order(4), type(UNIFORM_TYPE_GL_FLOAT) {}

                // 3D
                UniformAssignment(GLint x, GLint y, GLint z) : 
                    sx(x), sy(y), sz(z), order(3), type(UNIFORM_TYPE_GL_INT) {}
                UniformAssignment(GLuint x, GLuint y, GLuint z) :
                    ux(x), uy(y), uz(z), order(3), type(UNIFORM_TYPE_GL_UINT) {}
                UniformAssignment(GLfloat x, GLfloat y, GLfloat z) :
                    x(x), y(y), z(z), order(3), type(UNIFORM_TYPE_GL_FLOAT) {}
                
                // 2D
                UniformAssignment(GLint x, GLint y) : sx(x), sy(y), order(2), type(UNIFORM_TYPE_GL_INT) {}
                UniformAssignment(GLuint x, GLuint y) : ux(x), uy(y), order(2), type(UNIFORM_TYPE_GL_UINT) {}
                UniformAssignment(GLfloat x, GLfloat y) : x(x), y(y), order(2), type(UNIFORM_TYPE_GL_FLOAT) {}
                
                // 1D
                UniformAssignment(GLint x) : sx(x), order(1), type(UNIFORM_TYPE_GL_INT) {}
                UniformAssignment(GLuint x) : ux(x), order(1), type(UNIFORM_TYPE_GL_UINT) {}
                UniformAssignment(GLfloat x) : x(x), order(1), type(UNIFORM_TYPE_GL_FLOAT) {}
            };

            class UniformProxy
            {
                private:
                    GLint           uniform_loc;
                    GLint           prog;
                    std::string     name;

                public:
                    UniformProxy(const std::string& uniformname, int prog) : prog(prog), name(uniformname)
                    {
                        while (glGetError() != GL_NO_ERROR);

                        uniform_loc = glGetUniformLocation(prog, uniformname.c_str());

                        if (uniform_loc == -1)
                            std::abort();
                    }

                    void operator=(const UniformAssignment& val)
                    {
                        if (uniform_loc == -1)
                            return;

                        GLint oldprog = program_in_use;
                        if (prog != program_in_use)
                            glUseProgram(prog);

                        while(glGetError() != GL_NO_ERROR);
                        if (val.type == UNIFORM_TYPE_GL_INT)
                        {
                            if (val.order == 1) glUniform1i(uniform_loc, val.sx);
                            if (val.order == 2) glUniform2i(uniform_loc, val.sx, val.sy);
                            if (val.order == 3) glUniform3i(uniform_loc, val.sx, val.sy, val.sz);
                            if (val.order == 4) glUniform4i(uniform_loc, val.sx, val.sy, val.sz, val.sw);
                        } else if (val.type == UNIFORM_TYPE_GL_UINT) {
                            if (val.order == 1) glUniform1ui(uniform_loc, val.ux);
                            if (val.order == 2) glUniform2ui(uniform_loc, val.ux, val.uy);
                            if (val.order == 3) glUniform3ui(uniform_loc, val.ux, val.uy, val.uz);
                            if (val.order == 4) glUniform4ui(uniform_loc, val.ux, val.uy, val.uz, val.uw);
                        } else if (val.type == UNIFORM_TYPE_GL_FLOAT) {
                            if (val.order == 1) glUniform1f(uniform_loc, val.x);
                            if (val.order == 2) glUniform2f(uniform_loc, val.x, val.y);
                            if (val.order == 3) glUniform3f(uniform_loc, val.x, val.y, val.z);
                            if (val.order == 4) glUniform4f(uniform_loc, val.x, val.y, val.z, val.w);
                        }

                        if (glGetError() != GL_NO_ERROR)
                            std::abort();

                        if (oldprog != program_in_use)
                            glUseProgram(oldprog);
                    }
            };

        public:
            Shader_Base() {}
            virtual ~Shader_Base() 
            {
                if (program_in_use == program)
                    glUseProgram(0);
                if (program == 0)
                    return;

                glDeleteProgram(program);
            }

            void use(bool bind_textures = true);

            static const GLint current_program() { return program_in_use; }

            virtual void reload() = 0;
            virtual bool compile() = 0;

            inline UniformProxy operator[](const std::string& name)
            {
                return UniformProxy(name, program);
            }
    };

    class Shader : public Shader_Base
    {
        private:
        public:
            Shader() {}
           ~Shader() {}
            
            void reload();
            bool compile();
    };
}

#endif
