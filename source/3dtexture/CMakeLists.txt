file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.h")

add_executable(3dtexture ${SOURCES} ${HEADERS})

target_include_directories(3dtexture PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/glew-2.1.0/include)
target_include_directories(3dtexture PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/freeglut-3.6.0/include)
target_include_directories(3dtexture PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/glm-1.0.1)

target_link_libraries(3dtexture PRIVATE common)
target_link_libraries(3dtexture PRIVATE glew_s)
target_link_libraries(3dtexture PRIVATE freeglut_static)
