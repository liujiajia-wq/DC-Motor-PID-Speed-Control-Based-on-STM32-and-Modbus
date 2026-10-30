#-------------------------------------------------
#
# Project created by QtCreator 2026-05-07T10:05:58
#
#-------------------------------------------------

QT       += core gui serialport charts widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# 源码文件
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    plotwidget.cpp \
    serialportmanager.cpp

HEADERS += \
    mainwindow.h \
    plotwidget.h \
    serialportmanager.h

FORMS += mainwindow.ui

# 中文不乱码
DEFINES += QT_DEPRECATED_WARNINGS
