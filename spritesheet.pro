QT += core
QT += xml

TARGET = spritesheet
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app

SOURCES += main.cpp \
    plist/plistparser.cpp \
    plist/plistserializer.cpp \
    binPack/MaxRectsBinPack.cpp \
    imageTools/ImageTrim.cpp \
    Generator.cpp \
    binPack/Rect.cpp

HEADERS += \
    plist/plistparser.h \
    plist/plistserializer.h \
    binPack/MaxRectsBinPack.h \
    imageTools/ImageTrim.h \
    Generator.h \
    binPack/Rect.h \
    imageTools/imagerotate.h

