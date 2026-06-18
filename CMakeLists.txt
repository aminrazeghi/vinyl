cmake_minimum_required(VERSION 3.16)
project(groovr VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Qml Quick DBus)

# Add our source directories
include_directories(src)

# Declare target
qt_add_executable(groovr_app
    src/main.cpp
    src/mpris/mprisplayer.h src/mpris/mprisplayer.cpp
    src/mpris/mprismanager.h src/mpris/mprismanager.cpp
    src/settings/settingsmanager.h src/settings/settingsmanager.cpp
    src/mediacontroller/mediacontroller.h src/mediacontroller/mediacontroller.cpp
    src/admuter/admuter.h src/admuter/admuter.cpp
)

# Use modern Qt 6 QML module system
qt_add_qml_module(groovr_app
    URI GroovrApp
    VERSION 1.0
    QML_FILES
        src/qml/main.qml
        src/qml/GroovrRecord.qml
        src/qml/ControlBar.qml
        src/qml/SeekBar.qml
        src/qml/GlassPanel.qml
        src/qml/PlayerSelector.qml
    RESOURCES
        assets/default_cover.png
)

target_link_libraries(groovr_app PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::DBus
)

install(TARGETS groovr_app RUNTIME DESTINATION bin)
