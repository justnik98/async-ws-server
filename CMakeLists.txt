cmake_minimum_required(VERSION 3.16)
project(async_server)

set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(${PROJECT_NAME} main.cpp src/session.cpp src/session.h src/listener.cpp src/listener.h src/pipeline.cpp src/pipeline.h)

target_link_libraries(${PROJECT_NAME} ${CONAN_LIBS})