
cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE Profile)
#set(CMAKE_BUILD_TYPE RelWithDebInfo)

PROJECT( robot )
FIND_PACKAGE( OpenCV REQUIRED )
find_package (Threads)
find_package(BISON)
find_package(FLEX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

FLEX_TARGET(scanner yylex.l  ${CMAKE_CURRENT_BINARY_DIR}/yylex.cc)
BISON_TARGET(parser parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cc)
ADD_FLEX_BISON_DEPENDENCY(scanner parser)

ADD_EXECUTABLE( robot robot.cpp  libcam.cpp commthread.cpp camthread.cpp ${FLEX_scanner_OUTPUTS} ${BISON_parser_OUTPUTS})
TARGET_LINK_LIBRARIES( robot ${OpenCV_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${FLEX_LIBRARIES} ${BISON_LIBRARIES} -lconfig -lcvblob)
