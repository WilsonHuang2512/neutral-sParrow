cmake_minimum_required(VERSION 3.5)

add_compile_options(-fPIC)
#add_definitions(-std=c++11)

set(CMAKE_CXX_FLAGS "-Wl,-rpath=./")
find_package(OpenCV CONFIG REQUIRED)
project(example CXX)

set(APP_SRC cpp_example.cpp)


#print message
message(${PROJECT_SOURCE_DIR})   

add_executable(example cpp_example.cpp)

target_link_libraries(example
 ${CMAKE_SOURCE_DIR}/libcamera.so
 ${CMAKE_SOURCE_DIR}/libenumerate.so
	${OpenCV_LIBS}
	)

