#ifndef PLATFORM_H
#define PLATFORM_H

#include <QtCore>
#include "histogram.h"
#include "histogram_avx.h"

// define private struct
#ifdef Q_OS_LINUX

namespace platform {

struct FileInfoPrivate {
    bool isValid;
    qint64 mtime;
    qint64 ctime;
    qint64 atime;
    qint64 size;
    unsigned long device;
    enum { IsOther, IsFile, IsDirectory } mode;
};

} // namespace platform

#else // #ifdef Q_OS_LINUX
# include <QFileInfo>

namespace platform {
struct FileInfoPrivate {
    bool isValid;
    QFileInfo i;
};
} // namespace platform

#endif // #ifdef Q_OS_LINUX

// public interface
namespace platform {

class FileInfo {
    FileInfoPrivate m_priv;

public:
    FileInfo(QString &path);

    // returns true if the methods listed below return useful results
    bool isValid() const;

    qint64 createdTime();
    qint64 lastModified();
    qint64 lastAccess();
    qint64 size();
    bool isRegularFile();
    bool isDirectory();

    // returns true if this object is not on the same device as other object
    bool crossesMountpointFrom(FileInfo &other);
};

} // namespace platform


// FileInfo definition
#ifdef Q_OS_LINUX

namespace platform {
inline bool FileInfo::isValid() const
{
   return m_priv.isValid;
}

inline qint64 FileInfo::createdTime()
{
    return m_priv.ctime;
}

inline qint64 FileInfo::lastModified()
{
    return m_priv.mtime;
}

inline qint64 FileInfo::lastAccess()
{
    return m_priv.atime;
}

inline qint64 FileInfo::size()
{
    return m_priv.size;
}

inline bool FileInfo::isRegularFile()
{
    return (m_priv.mode == FileInfoPrivate::IsFile);
}

inline bool FileInfo::isDirectory()
{
    return (m_priv.mode == FileInfoPrivate::IsDirectory);
}
} // namespace platform

#else // #ifdef Q_OS_LINUX

namespace platform {
inline FileInfo::FileInfo(QString &path):
    m_priv{false, QFileInfo(path)}
{
    if (m_priv.i.exists()) {
        m_priv.isValid = true;
    }
}

inline bool FileInfo::isValid() const
{
   return m_priv.isValid;
}

inline qint64 FileInfo::createdTime()
{
    return m_priv.i.birthTime().toSecsSinceEpoch();
}

inline qint64 FileInfo::lastModified()
{
    return m_priv.i.lastModified().toSecsSinceEpoch();
}

inline qint64 FileInfo::lastAccess()
{
    return m_priv.i.lastRead().toSecsSinceEpoch();
}

inline qint64 FileInfo::size()
{
    return m_priv.i.size();
}

inline bool FileInfo::isRegularFile()
{
    return m_priv.i.isFile();
}

inline bool FileInfo::isDirectory()
{
    return m_priv.i.isDir();
}

inline bool FileInfo::crossesMountpointFrom(FileInfo &other)
{
    Q_UNUSED(other)
    return false;
}

} // namespace platform

#endif // #ifdef Q_OS_LINUX

namespace platform {

extern histogram::Impl *histogramImpl;
void initHistogramImpl();

} // namespace platform

#endif // PLATFORM_H
