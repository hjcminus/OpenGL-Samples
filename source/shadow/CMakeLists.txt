add_executable(shadow shadow.cpp)

target_include_directories(shadow PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/glew-2.1.0/include)
target_include_directories(shadow PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/freeglut-3.6.0/include)
target_include_directories(shadow PRIVATE $(PROJECT_SOURCE_DIR)/../../third_party/glm-1.0.1)

target_link_libraries(shadow PRIVATE common)
target_link_libraries(shadow PRIVATE glew_s)
target_link_libraries(shadow PRIVATE freeglut_static)

if (WIN32)
  # set using UNICODE characterset
  add_definitions(-DUNICODE -D_UNICODE)
endif()
