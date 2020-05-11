#include "dirage.h"
#include "scanner.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DirAge w;

    qRegisterMetaType<AgeVector>();
    qRegisterMetaType<AgeDatapoint>();
    w.show();
    return a.exec();
}
