TEMPLATE = app

# Project Version
VERSION = 0.3-alpha

VERSION_STR='\\"$${VERSION}\\"'
DEFINES += PROJECT_VERSION=\"$${VERSION_STR}\"

QT += qml quick bluetooth webview network
CONFIG += c++11

SOURCES += \
    src/main.cpp \
    src/onecontrol.cpp \
    src/settings.cpp \
    src/bluetooth.cpp \
    src/multiplexerhandler.cpp \
    src/bluetoothmultiplexer.cpp \
    src/jsonrpc.cpp \
    src/tcplistener.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    src/onecontrol.h \
    src/settings.h \
    src/bluetooth.h \
    src/multiplexerhandler.h \
    src/bluetoothmultiplexer.h \
    src/jsonrpc.h \
    src/tcplistener.h

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
