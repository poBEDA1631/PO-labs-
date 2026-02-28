cmake_minimum_required (VERSION 3.8)

# Політика для MSVC (залишаємо, як було в оригіналі для стабільності в VS)
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
endif()

project ("MyProject")

# Встановлюємо стандарт C++20 для всього проекту
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Створюємо ТРИ окремі програми (три кнопки запуску)
add_executable(task3 "task3.cpp" "MyProject.h")
add_executable(task4 "task4.cpp" "MyProject.h")
add_executable(task5 "task5.cpp" "MyProject.h")

# Якщо ви використовуєте потоки (pthread) у UNIX, додайте це:
if(UNIX)
    target_link_libraries(task3 pthread)
    target_link_libraries(task4 pthread)
    target_link_libraries(task5 pthread)
endif()