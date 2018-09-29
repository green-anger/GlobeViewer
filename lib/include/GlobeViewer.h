#pragma once

#include <memory>


namespace gv {


class GlobeViewer
{
public:
    GlobeViewer();
    ~GlobeViewer();

    bool validSetup() const;
    void render();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};


}
