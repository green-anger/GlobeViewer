#include <exception>
#include <fstream>
#include <sstream>
#include <iostream>

#include <LoadGl.h>
#include <Shader.h>


namespace gw {
namespace support {


    using std::string;
    using std::ifstream;
    using std::stringstream;
    using std::cout;
    using std::endl;

    ///// ----- ShaderImpl ----- //////////////////////////////////////////////////////////////////

    class Shader::ShaderImpl
    {
    public:

        class BadFile : public std::exception
        {
        };

        typedef enum {
            SHADER_OK,
            SHADER_ERROR_READ_FILE,
            SHADER_ERROR_VERTEX_BAD_FILE,
            SHADER_ERROR_GEOMETRY_BAD_FILE,
            SHADER_ERROR_FRAGMENT_BAD_FILE,
            SHADER_ERROR_VERTEX_COMPILE,
            SHADER_ERROR_GEOMETRY_COMPILE,
            SHADER_ERROR_FRAGMENT_COMPILE,
            SHADER_ERROR_PROGRAM_LINK
        } ShaderState;

        ShaderImpl( const string& vert, const string& frag, const string& geom = string() );

        ShaderState loadShader( const string& vert, const string& frag,
            const string& geom = string() );
        GLuint createShader( const string& fileName, GLenum type ) const;

        ShaderState state;
        GLuint program;
    };

    Shader::ShaderImpl::ShaderImpl( const string& vert, const string& frag, const string& geom )
        : state( loadShader( vert, frag, geom ) )
    {
    }

    Shader::ShaderImpl::ShaderState Shader::ShaderImpl::loadShader( const string& vert,
        const string& frag, const string& geom )
    {
        GLint success;
        GLchar infoLog[512];

        GLuint vertId;
        try {
            vertId = createShader( vert, GL_VERTEX_SHADER );
        }
        catch ( BadFile ) {
            return SHADER_ERROR_VERTEX_BAD_FILE;
        }
        glGetShaderiv( vertId, GL_COMPILE_STATUS, &success );
        if ( !success ) {
            glGetShaderInfoLog( vertId, 512, 0, infoLog );
            cout << "Error: vertex shader compilation failed\n" << infoLog << endl;
            return SHADER_ERROR_VERTEX_COMPILE;
        }

        GLuint fragId;
        try {
            fragId = createShader( frag, GL_FRAGMENT_SHADER );
        }
        catch ( BadFile ) {
            return SHADER_ERROR_FRAGMENT_BAD_FILE;
        }
        glGetShaderiv( fragId, GL_COMPILE_STATUS, &success );
        if ( !success ) {
            glGetShaderInfoLog( fragId, 512, 0, infoLog );
            cout << "Error: fragment shader compilation failed\n" << infoLog << endl;
            return SHADER_ERROR_FRAGMENT_COMPILE;
        }

        GLuint geomId;
        if ( !geom.empty() ) {
            try {
                geomId = createShader( geom, GL_GEOMETRY_SHADER );
            }
            catch ( BadFile ) {
                return SHADER_ERROR_GEOMETRY_BAD_FILE;
            }
            glGetShaderiv( geomId, GL_COMPILE_STATUS, &success );
            if ( !success ) {
                glGetShaderInfoLog( geomId, 512, 0, infoLog );
                cout << "Error: geometry shader compilation failed\n" << infoLog << endl;
                return SHADER_ERROR_GEOMETRY_COMPILE;
            }
        }

        program = glCreateProgram();
        glAttachShader( program, vertId );
        if ( !geom.empty() )
            glAttachShader( program, geomId );
        glAttachShader( program, fragId );
        glLinkProgram( program );
        glGetProgramiv( program, GL_LINK_STATUS, &success );
        if ( !success ) {
            glGetProgramInfoLog( program, 512, 0, infoLog );
            cout << "Error: shader program linkage failed\n" << infoLog << endl;
            return SHADER_ERROR_PROGRAM_LINK;
        }

        glDeleteShader( vertId );
        if ( !geom.empty() )
            glDeleteShader( geomId );
        glDeleteShader( fragId );

        return SHADER_OK;
    }

    GLuint Shader::ShaderImpl::createShader( const string& fileName, GLenum type ) const
    {
        string shaderCode;
        ifstream shaderFile;
        //shaderFile.exceptions ( ifstream::badbit );
        try {
            shaderFile.open( fileName.c_str() );
            if ( !shaderFile.good() )
            {
                shaderFile.close();
                throw BadFile();
            }
            stringstream shaderStream;
            shaderStream << shaderFile.rdbuf();
            shaderFile.close();
            shaderCode = shaderStream.str();
        }
        catch( ifstream::failure ) {
            cout << "Error: cannot read file " << fileName << endl;
        }
        GLuint shaderId;
        shaderId = glCreateShader( type );
        const GLchar* codep = shaderCode.c_str();
        glShaderSource( shaderId, 1, &codep, 0 );
        glCompileShader( shaderId );
        return shaderId;
    }

    ///// ----- Shader ----- //////////////////////////////////////////////////////////////////////

    Shader::Shader( const std::string& vert, const string& frag, const string& geom )
        : impl_( new ShaderImpl( vert, frag, geom ) )
    {
    }

    Shader::~Shader()
    {
    }

    bool Shader::isValid() const
    {
        return impl_->state == ShaderImpl::SHADER_OK;
    }

    void Shader::use() const
    {
        if ( !isValid() )
            return;
        glUseProgram( impl_->program );
    }

    int Shader::uniformLocation( const std::string& name ) const
    {
        return glGetUniformLocation( impl_->program, name.c_str() );
    }


}}
