#include "dirage.h"
#include "scanner.h"
#include "platform.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DirAge w;

    qRegisterMetaType<AgeVector>();
    platform::initHistogramImpl();
    w.show();
    return a.exec();
}
