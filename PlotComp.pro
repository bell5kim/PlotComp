#-------------------------------------------------
#
# Project created by QtCreator 2015-03-30T12:49:36
#
#-------------------------------------------------

QT       += core gui printsupport xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PlotComp
TEMPLATE = app

INCLUDEPATH += /usr/local/qwt-6.1.2/include/
LIBS += -L/usr/local/qwt-6.1.2/lib/ -lqwt

SOURCES += main.cpp\
        plotcomp.cpp \
    myplotcomp.cpp

HEADERS  += plotcomp.h \
    medianFilter.h \
    myplotcomp.h \
    pixmaps.h

FORMS    += plotcomp.ui

CONFIG   += qwt

DEFINES  += XVMC
