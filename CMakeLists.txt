CMAKE_MINIMUM_REQUIRED(VERSION 3.17)
PROJECT(t500_arm_desktop)
SET(CMAKE_CXX_STANDARD 20)

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})

add_executable(${PROJECT_NAME} main.cpp gui/ControllerWindow.cpp gui/ControllerWindow.h gui/ConnectWindow.cpp gui/ConnectWindow.h Driver.cpp Driver.h gui/dialogs.cpp gui/dialogs.h)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${GTK3_LIBRARIES})