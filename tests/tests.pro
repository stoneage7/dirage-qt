QT += testlib
QT += gui
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES +=  tst_histogram_test.cpp

INCLUDEPATH += \
    ../src/ \
    ../histogram_avx/ \
    ../histogram/

LIBS += \
    -L../histogram_avx/ -lhistogram_avx \
    -L../histogram/ -lhistogram
