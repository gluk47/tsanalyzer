#-------------------------------------------------
#
# Project created by QtCreator 2015-04-22T18:55:13
#
#-------------------------------------------------

QT       += core gui
CONFIG   += c++11
DEFINES  += _USE_MATH_DEFINES

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = GUI
TEMPLATE = app


SOURCES += main.cc\
        MainWindow.cc \
    TimeSeries.cc \
    AnalyzeWidget.cc \
    GenerateWidget.cc \
    CoefftWidget.cc \
    Plot.cc \
    helpers.cc \
    qcustomplot.cpp

HEADERS  += MainWindow.h \
    TimeSeries.h \
    AnalyzeWidget.h \
    GenerateWidget.h \
    CoefftWidget.h \
    Plot.h \
    helpers.h \
    qcustomplot.h

FORMS    += MainWindow.ui \
    AnalyzeWidget.ui \
    GenerateWidget.ui \
    CoefftWidget.ui \
    Plot.ui
