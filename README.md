# glDoom Resurrected (again!)
This repository contains improvements for glDoom port

### Fixes (original from REDPOWAR)
* new floor/ceiling generation code (similar to code from Doom Legacy port)
* aspect ratio fix (correct rendering on 16:9 displays)
* fixed invisible area culling (behind walls in sectors without ceiling)
* few bugfixes for buffer overflows & invalid memory pointers

### Fixes from Myself
* 64bit cleanups to allow it to build and run on 64bit systems
* Cleanups for modern types (uintptr_t, intptr_t and MSVC specific functions like (underscore)access etc..)
* Addition of the score system code from WINDOOM

### Future
I am planning for this port, the following (in order or priority)

1. Cleaning up and optimising the code to run properly on 64bit Windows (DONE)
2. Cleaning up the OpenGL code and moving some things to shaders
3. Making glDoom multiplatform (Linux and macOS) - with an SDL window but still using OpenGL to draw and accelerate
4. Allowing the user to select the resolution and fullscreen/window within glDoom's menu
5. Increasing the vanilla limits and adding support for No Rest For The Living and SIGIL

### Note - 16.05.2022
Current I am adding SDL2 Mixer support to stream audio (ported from various SDL2-based source ports).

Plus more!

### Why
The answer is simple. These older source ports are special, they paved the way for what we have now, an actual piece of Doom history. While glDoom will never be as popular as GZDoom or PRBoom, it's history and legacy is an important part of the Doom Community and rather than be forgotten, they should be maintained for the next generation to use.

### Compiling
Use the Visual Studio project and build for your architecture.

To use it, you will need to also create the gldoom.wad file by using the wadbuild tool (wadbuild.exe gldoom.lst) in the resources directory.
