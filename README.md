## What is GlobeViewer?

[GlobeViewer](https://github.com/green-anger/GlobeViewer) allows you to view popular online maps not in Mercator but Orthographic projection.
[Greenland will never be larger than Australia](https://www.quora.com/Which-is-bigger-Australia-or-Greenland) again.
In addition to viewing tiles GlobeViewer caches them. So viewed once the tiles will be available to you afterwards even offline!
Currently it supports [OpenStreetMap](https://www.openstreetmap.org) and [2GIS](https://2gis.com) tiles.

Keep in mind that the tiles are not changed, so switching to another projection (from Mercator to Orthographic) will distort them a bit.
But all symbols remain readable at all zoom levels.

## How to use it

GlobeViewer is designed to be used as a library in any application than can create OpenGL context. Currently it has only been tested with [GLFW](https://www.glfw.org/).
You can find corresponding example [next door](./examples/glfw).

If you have build [the library](./lib) (and all its dependencies in case of static build), you can easily use it in your project by adding single header [GlobeViewer.h](/lib/include/GlobeViewer.h).
And that's it. Just call API methods from your application.

Note: _currently the library is poorly documented (if at all), please see provided example to grasp the subject_.

## Provided example controls

[GLFW example](./examples/glfw) has the following controls:
* Hold left mouse button and drag to move the Globe
* Hold left mouse button, hold Shift key and drag to rotate the Globe
* Scroll mouse wheel up to zoom in
* Scroll mouse wheel down to zoom out
* Press key 'C' to place the Globe in window center
* Press key 'B' to rotate the Globe so that projection center is at [0, 0]
* Press key 'P' to rotate the Globe so that projection center is at the current mouse cursor position if it's inside the Globe, ignore otherwise
* Press key 'T' to toggle between two tile servers
* Press key '1' to toggle display of wire-frame
* Press key '2' to toggle display of map tiles
* Press Escape to exit

## Screenshots

![The Globe](./doc/images/globe.png "The Globe")
![The North Pole](./doc/images/north_pole.png "The North Pole")
![The South Pole](./doc/images/south_pole.png "The South Pole")
![Melbourne from far away](./doc/images/melbourne_far.png "Melbourne from far away")
![Melbourne from not so far away](./doc/images/melbourne_mid.png "Melbourne from not so far away")
![Melbourne at zoom level 19](./doc/images/melbourne_close.png "Melbourne at zoom level 19")

## Dependencies

General:
* C++14

Embedded into the source code:
* [Glad OpenGL loader](https://github.com/Dav1dde/glad) files generated on [this site](https://glad.dav1d.de/)
* [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h) from [stb repo](https://github.com/nothings/stb/)
* [ThreadSafePrinter](https://github.com/green-anger/ThreadSafePrinter/)
* [Profiler](https://github.com/green-anger/Profiler/)

Library
* [OpenGL](https://www.khronos.org/opengl/)
* [Boost](https://www.boost.org/) 1.67.0
    - [Boost.Asio](https://www.boost.org/doc/libs/release/libs/asio/)
    - [Boost.Beast](https://www.boost.org/doc/libs/1_68_0/libs/beast/)
    - [Boost.Filesystem](https://www.boost.org/doc/libs/release/libs/filesystem/)
    - [Boost.Optional](https://www.boost.org/doc/libs/release/libs/optional/)
    - [Boost.Signals2](https://www.boost.org/doc/libs/release/libs/signals2/)
    - [Boost.System](https://www.boost.org/doc/libs/release/libs/system/)
* [GLM](https://glm.g-truc.net/)
* [PROJ.4](https://github.com/OSGeo/proj.4/) 5.0.1

GLFW example:
* [GLFW](https://www.glfw.org/)

## LICENSE

GlobeViewer is licensed under [MIT License](./LICENSE).
