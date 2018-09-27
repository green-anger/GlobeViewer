#pragma once

#include <memory>

#include "LoadGl.h"


namespace gw {


    namespace support {
        class Shader;
    }


    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        void render();

    private:
        std::unique_ptr<support::Shader> shaderSimple_;
        GLint ssProj_;
        GLint ssColor_;
    };


}
