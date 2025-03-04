cmake_minimum_required(VERSION 3.10)

project (gl_samples 
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# set output binary file directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

source_group(TREE ${PROJECT_BINARY_DIR})

# add options defined in freeglut-3.6.0/CMakeLists.txt
option(FREEGLUT_BUILD_SHARED_LIBS "Build FreeGLUT shared library." ON)
option(FREEGLUT_BUILD_DEMOS "Build FreeGLUT demos." ON )

set(FREEGLUT_BUILD_SHARED_LIBS OFF)	# do not compile shared lib
set(FREEGLUT_BUILD_DEMOS OFF)		# do not compile glut demos

add_subdirectory(third_party/freeglut-3.6.0)

option (BUILD_UTILS "utilities" ON)

set(BUILD_UTILS OFF)	# exclude glew utils
add_definitions(-DGLEW_USE_STATIC_LIBS)

add_subdirectory(third_party/glew-2.1.0/build/cmake)
add_subdirectory(source/common)
add_subdirectory(source/glow)
add_subdirectory(source/depth_peeling)
add_subdirectory(source/dual_depth_peeling)
add_subdirectory(source/3dtexture)
