# CMakeList.txt: проект CMake для lab5
#

# Додаємо виконуваний файл
add_executable(lab5 "lab5.cpp")

# Стандарт C++20 (для std::optional та ін.)
if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET lab5 PROPERTY CXX_STANDARD 20)
endif()

# Лінковка бібліотеки потоків (потрібна для std::thread на Linux/macOS)
find_package(Threads REQUIRED)
target_link_libraries(lab5 PRIVATE Threads::Threads)

# Встановлення робочої директорії при запуску з IDE —
# щоб відносний шлях "./static" вказував на корінь проекту, а не на папку збірки.
set_target_properties(lab5 PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)
