#-------------------------------------------------
#
# Project created by QtCreator 2023-02-27T09:38:15
#
#-------------------------------------------------

QT       += core gui
QT         +=network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MyChatClient
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp

HEADERS  += widget.h

FORMS    += widget.ui
