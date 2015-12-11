QT += core
QT += xml

TARGET = spriteglue
CONFIG += console
CONFIG += c++11

TEMPLATE = app

SOURCES += main.cpp \
    plist/plistserializer.cpp \
    binPack/MaxRectsBinPack.cpp \
    imageTools/ImageTrim.cpp \
    Generator.cpp \
    binPack/Rect.cpp \
    ImageSorter.cpp

HEADERS += \
    plist/plistserializer.h \
    binPack/MaxRectsBinPack.h \
    imageTools/ImageTrim.h \
    Generator.h \
    binPack/Rect.h \
    imageTools/imagerotate.h \
    ImageSorter.h

