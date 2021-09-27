QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    convert.cpp \
    jpegoptions.cpp \
    lodepng/lodepng.cpp \
    main.cpp \
    mainwindow.cpp \

HEADERS += \
    convert.h \
    jpegoptions.h \
    lodepng/lodepng.h \
    mainwindow.h

FORMS += \
    jpegoptions.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



win32: LIBS += -LD:/vcpkg/packages/x265_x64-windows/lib/ -llibx265

INCLUDEPATH += D:/vcpkg/packages/x265_x64-windows/include
INCLUDEPATH += D:/vcpkg/packages/x265_x64-windows/bin
DEPENDPATH += D:/vcpkg/packages/x265_x64-windows/include

win32: LIBS += -LD:/vcpkg/packages/libde265_x64-windows/lib/ -llibde265

INCLUDEPATH += D:/vcpkg/packages/libde265_x64-windows/include
INCLUDEPATH += D:/vcpkg/packages/libde265_x64-windows/bin
DEPENDPATH += D:/vcpkg/packages/libde265_x64-windows/include

win32: LIBS += -LD:/vcpkg/packages/libheif_x64-windows/lib/ -lheif

INCLUDEPATH += D:/vcpkg/packages/libheif_x64-windows/include
INCLUDEPATH += D:/vcpkg/packages/libheif_x64-windows/bin
DEPENDPATH += D:/vcpkg/packages/libheif_x64-windows/include



win32: LIBS += -LD:/vcpkg/packages/libjpeg-turbo_x64-windows/lib/ -ljpeg

win32: LIBS += -LD:/vcpkg/packages/libjpeg-turbo_x64-windows/lib/ -lturbojpeg

INCLUDEPATH += D:/vcpkg/packages/libjpeg-turbo_x64-windows/include
INCLUDEPATH += D:/vcpkg/packages/libjpeg-turbo_x64-windows/bin
DEPENDPATH += D:/vcpkg/packages/libjpeg-turbo_x64-windows/include

RC_ICONS = icon.ico





