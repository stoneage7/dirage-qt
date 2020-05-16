
#include "platform.h"

#ifdef Q_OS_LINUX
#include "sys/stat.h"
#include <cstring>

namespace platform {

FileInfo::FileInfo(QString &path)
{
    QByteArray tmp = path.toUtf8();
    const char *cpath = tmp.data();
    struct stat buf;

    int e = lstat(cpath, &buf);
    if (e == 0) {
        m_priv.mtime = buf.st_mtim.tv_sec;
        m_priv.ctime = buf.st_ctim.tv_sec;
        m_priv.atime = buf.st_atim.tv_sec;
        m_priv.size = buf.st_size;
        if (S_ISDIR(buf.st_mode)) {
            m_priv.mode = FileInfoPrivate::IsDirectory;
        } else if (S_ISREG(buf.st_mode)) {
            m_priv.mode = FileInfoPrivate::IsFile;
        } else {
            m_priv.mode = FileInfoPrivate::IsOther;
        }
        m_priv.device = buf.st_dev;
        m_priv.isValid = true;
    } else {
        m_priv.mtime = -1;
        m_priv.ctime = -1;
        m_priv.atime = -1;
        m_priv.size = 0;
        m_priv.device = 0;
        m_priv.isValid = false;
    }
}

bool FileInfo::crossesMountpointFrom(FileInfo &other)
{
    return (other.m_priv.device != m_priv.device);
}


} // namespace platform
#endif // #ifdef Q_OS_LINUX



namespace platform {

histogram::Impl *histogramImpl;
static histogram::ScalarImpl histScalarImpl;
static histogram::AVX2Impl histAVX2Impl;

void initHistogramImpl()
{
#if defined(__GNUC__)
    __builtin_cpu_init();
    if (__builtin_cpu_supports("avx2")) {
        histogramImpl = &histScalarImpl;
    } else {
        histogramImpl = &histAVX2Impl;
    }
#else
    histogramImpl = new histogram::ScalarImpl;
#endif
}

} // namespace platform
