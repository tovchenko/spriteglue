QT += core
QT += xml

TARGET = spritesheet
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

TEMPLATE = app

SOURCES += main.cpp \
    binPack/Rect.cpp \
    plist/plistparser.cpp \
    plist/plistserializer.cpp \
    binPack/MaxRectsBinPack.cpp \
    imageTools/ImageCombine.cpp \
    imageTools/ImageTrim.cpp

HEADERS += \
    binPack/Rect.h \
    plist/plistparser.h \
    plist/plistserializer.h \
    binPack/MaxRectsBinPack.h \
    imageTools/ImageCombine.h \
    imageTools/ImageTrim.h

