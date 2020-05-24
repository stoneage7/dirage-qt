QT += testlib
QT += gui
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

INCLUDEPATH += \
    ../src \

SOURCES +=  tst_histogram_test.cpp


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../histogram_avx/release/ -lhistogram_avx
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../histogram_avx/debug/ -lhistogram_avx
else:unix: LIBS += -L$$OUT_PWD/../histogram_avx/ -lhistogram_avx

INCLUDEPATH += $$PWD/../histogram_avx
DEPENDPATH += $$PWD/../histogram_avx

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../histogram_avx/release/libhistogram_avx.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../histogram_avx/debug/libhistogram_avx.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../histogram_avx/release/histogram_avx.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../histogram_avx/debug/histogram_avx.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../histogram_avx/libhistogram_avx.a

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../histogram/release/ -lhistogram
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../histogram/debug/ -lhistogram
else:unix: LIBS += -L$$OUT_PWD/../histogram/ -lhistogram

INCLUDEPATH += $$PWD/../histogram
DEPENDPATH += $$PWD/../histogram

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../histogram/release/libhistogram.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../histogram/debug/libhistogram.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../histogram/release/histogram.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../histogram/debug/histogram.lib
else:unix: PRE_TARGETDEPS += $$OUT_PWD/../histogram/libhistogram.a

