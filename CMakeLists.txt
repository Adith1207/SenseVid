cmake_minimum_required (VERSION 2.8.7)

project (./sensevid;)

if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=gnu++0x")
endif()

find_package (OpenCV REQUIRED)

add_executable (./sensevid ./src/sensevid.cc ./src/parameters.cc ./src/video.cc ./src/mainFrame.cc ./src/secondFrame.cc ./src/dct.cc ./src/entropy.cc ./src/utils.cc)
target_link_libraries (./sensevid ${OpenCV_LIBS})

set(CMAKE_BUILD_TYPE Debug)
