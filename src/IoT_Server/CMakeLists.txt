CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT("IoT_Server")

SET(CMAKE_BUILD_TYPE RELEASE)
SET(CMAKE_VERBOSE_MAKEFILE true)

SET(OUTPUT_ELF "${CMAKE_PROJECT_NAME}.out")
SET(SRC_FILES main.cpp Client.cpp Device.cpp LocalDB.cpp Network.cpp
    ODBC_Manager.cpp Server.cpp tinyxml2.cpp User.cpp)

SET(CMAKE_C_COMPILER "g++")

LINK_LIBRARIES(pthread iodbc)
LINK_DIRECTORIES(/home/ubuntu/iodbc/lib/)

ADD_EXECUTABLE(${OUTPUT_ELF} ${SRC_FILES})
