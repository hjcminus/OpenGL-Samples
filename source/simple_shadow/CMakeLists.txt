add_executable(simple_shadow simple_shadow.cpp)

target_include_directories(simple_shadow PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/glew-2.1.0/include)
target_include_directories(simple_shadow PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/freeglut-3.6.0/include)
target_include_directories(simple_shadow PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/glm-1.0.1)

target_link_libraries(simple_shadow PRIVATE common)
target_link_libraries(simple_shadow PRIVATE glew_s)
target_link_libraries(simple_shadow PRIVATE freeglut_static)
