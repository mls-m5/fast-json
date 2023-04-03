cmake_minimum_required(VERSION 3.20)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif()


project(json_parser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(json_parser_lib INTERFACE)
target_include_directories(json_parser_lib INTERFACE include)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} json_parser_lib)

add_subdirectory(lib)
add_subdirectory(test)
