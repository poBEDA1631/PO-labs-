# CMakeLists.txt for Client1 executable
add_executable (Client1 "client1.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Client1 PROPERTY CXX_STANDARD 20)
endif()

# Link the shared Protocol library
target_link_libraries(Client1 PRIVATE Protocol)
