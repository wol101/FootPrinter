#-------------------------------------------------
#
# Project created by QtCreator 2021-12-31T16:28:30
#
#-------------------------------------------------

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FootPrinter
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

CONFIG += c++14
QMAKE_CXXFLAGS += -pthread
LIBS += -pthread
INCLUDEPATH += ../CppGPIO/include ../pystring

SOURCES += \
    main.cpp \
    FootPrinterMainDialog.cpp \
    ../CppGPIO/src/buttons.cpp \
    ../CppGPIO/src/gpio.cpp \
    ../CppGPIO/src/i2c.cpp \
    ../CppGPIO/src/lcd.cpp \
    ../CppGPIO/src/output.cpp \
    ../CppGPIO/src/tools.cpp \
    NAU7802.cpp \
    ../pystring/pystring.cpp \
    GPSAccess.cpp \
    UBloxControl.cpp \
    MotorControl.cpp \
    Preferences.cpp

HEADERS += \
    FootPrinterMainDialog.h \
    ../CppGPIO/src/make_unique.hpp \
    ../CppGPIO/src/tools.hpp \
    ../CppGPIO/include/cppgpio/buttons.hpp \
    ../CppGPIO/include/cppgpio/gpio.hpp \
    ../CppGPIO/include/cppgpio/i2c.hpp \
    ../CppGPIO/include/cppgpio/lcd.hpp \
    ../CppGPIO/include/cppgpio/output.hpp \
    ../CppGPIO/include/cppgpio.hpp \
    NAU7802.h \
    ../pystring/pystring.h \
    GPSAccess.h \
    UBloxControl.h \
    MotorControl.h \
    Preferences.h

FORMS += \
        FootPrinterMainDialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Resources.qrc
