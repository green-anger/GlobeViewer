#pragma once

#include <memory>
#include <string>


namespace gv {
namespace support {


    class Shader
    {
    public:
        Shader( const std::string& vert, const std::string& frag,
            const std::string& geom = std::string() );
        ~Shader();

        bool isValid() const;
        void use() const;
        int uniformLocation( const std::string& ) const;

    private:
        class ShaderImpl;
        std::unique_ptr<ShaderImpl> impl_;
    };


}}
