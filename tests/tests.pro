QT += testlib
QT += gui
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

INCLUDEPATH += ../src

SOURCES +=  tst_histogram_test.cpp \
    ../src/platform.cpp

