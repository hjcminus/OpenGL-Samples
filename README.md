# OpenGL-Samples

Some basic samples of OpenGL.


These samples can compile on Windows and Linux.

## Build

### Build on Windows

Use CMake to generate the solution file for Visual Studio C++.

Enter the OpenGL-Samples folder and create a new folder, e.g., msvc.

> mkdir msvc

Enter newly created folder
> cd msvc

Run the cmake command to generate the solution file gl_samples.sln.
> cmake ..

Now you can open the generated solution file with Visual Studio C++.

### Build on Linux

Use CMake to generate a Makefile for G++.

Enter the OpenGL-Samples folder and create a new folder, e.g., gmake2.

$ mkdir -p gmake2

Enter newly created folder

$ cd gmake2

Run the cmake command to generate the Makefile
> cmake ..

Type make to compile all the projects.

> make

