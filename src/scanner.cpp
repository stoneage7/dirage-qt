#include "scanner.h"
#include "platform.h"
#include <stack>
#include <QDir>
#include <QDirIterator>

SubdirsScanner::SubdirsScanner(const QString &path, QObject *parent):
    QThread(parent),
    m_path(path),
    m_ping(false)
{
    connect(this, &QThread::finished, this, &QObject::deleteLater);
}

// QDirIterator is not a QObject, so it should be fine to use std::
typedef std::stack<std::unique_ptr<QDirIterator> > QDirIteratorStack;
static void stackNewDirIterator(QDirIteratorStack &stack, const QString &entry)
{
    QDirIterator *tmp = new QDirIterator(entry, QDir::Dirs | QDir::Files |
                                         QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    stack.push(std::unique_ptr<QDirIterator>(tmp));
}

void SubdirsScanner::run()
{
    QEventLoop events;
    QDirIterator one_level_iter(m_path, QDir::Dirs | QDir::Files | QDir::Hidden |
                                QDir::NoDotAndDotDot | QDir::NoSymLinks);
    AgeVector toplevel_vector;

    while (one_level_iter.hasNext()) {
        QString toplevel_entry = one_level_iter.next();
        platform::FileInfo toplevel_info(toplevel_entry);
        if (toplevel_info.isDirectory()) {
            AgeVector subdir_vector;
            QDirIteratorStack stack;
            stackNewDirIterator(stack, toplevel_entry);

            while (!stack.empty()) {
                if (!stack.top()->hasNext()) {
                    stack.pop();
                    continue;
                }

                QString entry = stack.top()->next();
                platform::FileInfo info(entry);
                if (info.isValid()) {
                    if (info.isRegularFile()) {
                        this->processFile(subdir_vector, info.size(), info.lastModified());
                    } else if(info.isDirectory() && !info.crossesMountpointFrom(toplevel_info)) {
                        stackNewDirIterator(stack, entry);
                    }
                } else {
                    // TODO maybe give up this directory?
                }

                events.processEvents();
                if (this->isInterruptionRequested()) {
                    emit interrupted();
                    return;
                }
                if (m_ping) {
                    emit pong(entry);
                    m_ping = false;
                }
            }
            subdir_vector.finalize();
            emit haveSubDir(toplevel_entry, subdir_vector);
        } else if (toplevel_info.isRegularFile()) {
            events.processEvents();
            if (this->isInterruptionRequested()) {
                emit interrupted();
                return;
            }
            if (m_ping) {
                emit pong(toplevel_entry);
                m_ping = false;
            }
            if (toplevel_info.isValid() && toplevel_info.isRegularFile()) {
                this->processFile(toplevel_vector, toplevel_info.size(), toplevel_info.lastModified());
            }
        }
    }
    toplevel_vector.finalize();
    emit haveRootDir(m_path, toplevel_vector);
    qInfo() << "scan finished";
}

inline void SubdirsScanner::processFile(AgeVector &vector, qint64 size, qint64 timestamp)
{
    histogram::Datapoint dp;
    dp.key = timestamp;
    dp.value = size;
    vector.append(dp);
}

void SubdirsScanner::ping()
{
    m_ping = true;
}




