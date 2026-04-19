# CMakeLists.txt for Server executable
add_executable (Server "server.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Server PROPERTY CXX_STANDARD 20)
endif()

# Link the shared Protocol library
target_link_libraries(Server PRIVATE Protocol)
