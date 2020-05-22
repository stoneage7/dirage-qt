QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    agehistogram.cpp \
    ageitemdelegate.cpp \
    agemodel.cpp \
    agetableview.cpp \
    main.cpp \
    dirage.cpp \
    platform.cpp \
    scanner.cpp

HEADERS += \
    agehistogram.h \
    ageitemdelegate.h \
    agemodel.h \
    agetableview.h \
    dirage.h \
    platform.h \
    scanner.h

FORMS += \
    dirage.ui

#QMAKE_CXXFLAGS += -mavx2 -O3 -march=native

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    TODO

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

