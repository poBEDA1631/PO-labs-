# CMakeLists.txt for Protocol library (Header-only)
add_library(Protocol INTERFACE)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  target_compile_features(Protocol INTERFACE cxx_std_20)
endif()

# Make the include path available to targets linking to Protocol
target_include_directories(Protocol INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
