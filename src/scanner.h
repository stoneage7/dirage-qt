#ifndef SCANNER_H
#define SCANNER_H

#include <QObject>
#include <QThread>
#include "agehistogram.h"



class SubdirsScanner : public QThread
{
    Q_OBJECT

public:
    SubdirsScanner(const QString &path, QObject *parent = nullptr);
    void run() override;
    QString path() const { return m_path; }

private:
    QString m_path;
    bool m_ping;
    static void processFile(AgeVector &vector, qint64 size, qint64 timestamp);

signals:
    void haveSubDir(QString path, AgeVector datapoints);
    void haveRootDir(QString path, AgeVector datapoints);
    void interrupted();
    void pong(QString path);

public slots:
    void ping();
};

#endif // SCANNER_H
