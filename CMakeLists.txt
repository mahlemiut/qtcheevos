cmake_minimum_required(VERSION 3.10)

project(QtCheevos LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/network.cpp
    src/iconlist.cpp
)

set(HEADERS
    src/mainwindow.h
    src/network.h
    src/iconlist.h
)

set(UI_FILES
    src/mainwindow.ui
)

#qt6_wrap_ui(UI_HEADERS ${UI_FILES})

if(WIN32)
    add_definitions(-DWIN32)
endif()

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${UI_HEADERS})

target_link_libraries(${PROJECT_NAME} PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)