cmake_minimum_required(VERSION 3.16)

project(BowlingManagement VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt5 components (changed from Qt6 to Qt5)
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Network Sql)

# Enable automatic handling of Qt's MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Source files
set(SOURCES
    main.cpp
    MainWindow.cpp
    Actions.cpp
    EventBus.cpp
    LaneServer.cpp
    EnhancedLaneWidget.cpp
    GameDisplayDialog.cpp
    QuickStartDialog.cpp
    QuickGameDialog.cpp
    LeagueGameDialog.cpp
    NewBowlerDialog.cpp
    TeamManagementDialog.cpp
    LeagueManagementDialog.cpp
    DatabaseManager.cpp
    BowlerManagementDialog.cpp
    CalendarDialog.cpp
    DatabaseBrowserDialog.cpp
    LeagueScheduleDialog.cpp
    LeagueManager.cpp
)

# Header files
set(HEADERS
    MainWindow.h
    Actions.h
    EventBus.h
    LaneServer.h
    EnhancedLaneWidget.h
    GameDisplayDialog.h
    QuickStartDialog.h
    QuickGameDialog.h
    LeagueGameDialog.h
    NewBowlerDialog.h
    TeamManagementDialog.h
    LeagueManagementDialog.h
    DatabaseManager.h
    BowlerManagementDialog.h
    CalendarDialog.h
    DatabaseBrowserDialog.h
    LeagueScheduleDialog.h
    LeagueManager.h
)

# Create the executable
add_executable(BowlingManagement ${SOURCES} ${HEADERS})

# Link Qt5 libraries (changed from Qt6:: to Qt5::)
target_link_libraries(BowlingManagement
    Qt5::Core
    Qt5::Widgets
    Qt5::Network
    Qt5::Sql
)

# Set target properties
set_target_properties(BowlingManagement PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# Installation
install(TARGETS BowlingManagement
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)