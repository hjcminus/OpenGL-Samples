add_executable(tessellation tessellation.cpp)

target_include_directories(tessellation PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/glew-2.1.0/include)
target_include_directories(tessellation PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/freeglut-3.6.0/include)
target_include_directories(tessellation PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/glm-1.0.1)

target_link_libraries(tessellation PRIVATE common)
target_link_libraries(tessellation PRIVATE glew_s)
target_link_libraries(tessellation PRIVATE freeglut_static)
