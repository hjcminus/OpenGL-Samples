add_executable(glow glow.cpp)

target_include_directories(glow PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/glew-2.1.0/include)
target_include_directories(glow PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/freeglut-3.6.0/include)
target_include_directories(glow PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/glm-1.0.1)

target_link_libraries(glow PRIVATE common)
target_link_libraries(glow PRIVATE glew_s)
target_link_libraries(glow PRIVATE freeglut_static)
