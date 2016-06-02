TEMPLATE = app

# Project Version
VERSION = 0.1

VERSION_STR='\\"$${VERSION}\\"'
DEFINES += PROJECT_VERSION=\"$${VERSION_STR}\"

QT += qml quick
CONFIG += c++11

SOURCES += \
    src/main.cpp \
    src/onecontrol.cpp \
    src/settings.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    src/onecontrol.h \
    src/settings.h
