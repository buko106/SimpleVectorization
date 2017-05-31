cmake_minimum_required(VERSION 2.8)

set(CMAKE_CXX_COMPILER g++-5)
set(CMAKE_CXX_FLAGS "-std=c++14 -O0 -g -Wall -Wextra -Wconversion")

file(GLOB SRC src/*.cpp)
add_executable(main src/main.cpp ${SRC})

find_package(OpenCV REQUIRED)
FIND_PACKAGE( Boost COMPONENTS program_options REQUIRED )
target_link_libraries(main ${OpenCV_LIBS})
target_link_libraries(main ${Boost_LIBRARIES})
