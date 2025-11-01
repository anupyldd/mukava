cmake_minimum_required(VERSION 4.0)
project(mukava3d)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIB_NAME mukavalib)
add_library(${LIB_NAME})
target_sources(${LIB_NAME}
        PUBLIC
        FILE_SET CXX_MODULES
        FILES
        src/core/math.ixx
        src/core/types.ixx
        src/core/test.ixx
)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIB_NAME})