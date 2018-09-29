#pragma once

#include <memory>

#include "LoadGl.h"


namespace gw {


namespace support {
    class Shader;
}
class DataKeeper;
class Viewport;


class Renderer
{
public:
    explicit Renderer( const std::shared_ptr<const DataKeeper>&, const std::shared_ptr<const Viewport>& );
    ~Renderer();

    void render();

private:
    std::shared_ptr<const DataKeeper> dataKeeper_;
    std::shared_ptr<const Viewport> viewport_;

    std::unique_ptr<support::Shader> shaderSimple_;
    GLint ssProj_;
    GLint ssColor_;
};


}
