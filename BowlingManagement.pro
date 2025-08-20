QT += core widgets network sql

CONFIG += c++17

TARGET = BowlingManagement
TEMPLATE = app

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    Actions.cpp \
    EventBus.cpp \
    LaneServer.cpp \
    LaneWidget.cpp \
    QuickStartDialog.cpp \
    QuickGameDialog.cpp \
    LeagueGameDialog.cpp \
    NewBowlerDialog.cpp \
    BowlerInfoDialog.cpp \
    TeamManagementDialog.cpp \
    LeagueManagementDialog.cpp \
    DatabaseManager.cpp

HEADERS += \
    MainWindow.h \
    Actions.h \
    EventBus.h \
    LaneServer.h \
    LaneWidget.h \
    QuickStartDialog.h \
    QuickGameDialog.h \
    LeagueGameDialog.h \
    NewBowlerDialog.h \
    BowlerInfoDialog.h \
    TeamManagementDialog.h \
    LeagueManagementDialog.h \
    DatabaseManager.h

# Add resources if needed
# RESOURCES += resources.qrc

# Set output directories
DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
UI_DIR = $$PWD/build/ui
RCC_DIR = $$PWD/build/rcc