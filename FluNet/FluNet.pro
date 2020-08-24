#-------------------------------------------------
#
# Project created by QtCreator 2020-08-20T07:09:06
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FluNet
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += /usr/local/include/opencv4

#LIBS += $(shell pkg-config opencv --libs) /usr/lib/arm-linux-gnueabihf/libusb-1.0.so
LIBS += /usr/lib/arm-linux-gnueabihf/libusb-1.0.so
LIBS += /usr/local/lib/libopencv_objdetect.so
LIBS += /usr/local/lib/libopencv_calib3d.so
LIBS += /usr/local/lib/libopencv_highgui.so
LIBS += /usr/local/lib/libopencv_imgproc.so
LIBS += /usr/local/lib/libopencv_features2d.so
LIBS += /usr/local/lib/libopencv_calib3d.so
LIBS += /usr/local/lib/libopencv_core.so
LIBS += /usr/local/lib/libopencv_imgcodecs.so



CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    SeekCam.cpp \
    SeekDevice.cpp \
    SeekThermal.cpp \
    SeekThermalPro.cpp

HEADERS += \
        mainwindow.h \
    seek.h \
    SeekCam.h \
    SeekLogging.h \
    SeekDevice.h \
    SeekThermal.h \
    SeekThermalPro.h

FORMS += \
        mainwindow.ui

#unix {
   # CONFIG += link_pkgconfig
   # PKGCONFIG += opencv
#}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
