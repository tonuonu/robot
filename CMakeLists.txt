
cmake_minimum_required(VERSION 2.8)

PROJECT( robot )
FIND_PACKAGE( OpenCV REQUIRED )
find_package (Threads)

ADD_EXECUTABLE( robot robot.c )
TARGET_LINK_LIBRARIES( robot ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT})
