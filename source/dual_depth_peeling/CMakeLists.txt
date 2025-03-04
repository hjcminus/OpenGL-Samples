add_executable(dual_depth_peeling dual_depth_peeling.cpp)

target_include_directories(dual_depth_peeling PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/glew-2.1.0/include)
target_include_directories(dual_depth_peeling PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/freeglut-3.6.0/include)
target_include_directories(dual_depth_peeling PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/glm-1.0.1)

target_link_libraries(dual_depth_peeling PRIVATE common)
target_link_libraries(dual_depth_peeling PRIVATE glew_s)
target_link_libraries(dual_depth_peeling PRIVATE freeglut_static)
