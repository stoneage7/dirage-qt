
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

#ifdef __AVX2__
# define DIRAGE_HIST_VECTORIZE
# include "immintrin.h"
# include <memory>
#endif


// skip all inputs until keys (=timestamps) are >= the minimum
static inline bool
hist_skip_front_single_step(const Datapoint *vector, size_t num_datapoints,
                            qint64 min_key, size_t vec_i, size_t &tmp_i, size_t &step)
{
    step = num_datapoints >> 1;
    tmp_i = vec_i + step;
    return vector[tmp_i].key < min_key;
}

static inline size_t
hist_skip_front_single(const Datapoint *vector, size_t num_datapoints,
                       qint64 min_key, size_t vec_i)
{
    num_datapoints -= vec_i;
    min_key -= 1;
    size_t tmp_i = 0;
    while (num_datapoints > 0) {
        size_t step = 0;
        if (hist_skip_front_single_step(vector, num_datapoints, min_key, vec_i, tmp_i, step)) {
            vec_i = (tmp_i + 1);
            num_datapoints -= (step + 1);
        } else {
            num_datapoints = step;
        }
    }
    return vec_i;
}

#ifdef DIRAGE_HIST_VECTORIZE
// compare as unsigned 64bit integer.
static inline __m256i my_cmp_gt_epu64(__m256i a, __m256i b)
{
    __m256i xormask = _mm256_set1_epi64x(1LL<<63);
    a = _mm256_xor_si256(a, xormask);
    b = _mm256_xor_si256(b, xormask);
    return _mm256_cmpgt_epi64(a, b);
}

static inline __m256i my_cmp_lt_epu64(__m256i a, __m256i b)
{
    return my_cmp_gt_epu64(b, a);
}

static inline unsigned my_clz(unsigned c)
{
    unsigned i = 0;
    while (i < 32 && 0 == (c & (1 << (31-i)))) {
        i++;
    }
    return i;
}


void hist_skip_front_avx_step(const Datapoint *vector, size_t &num_datapoints,
                              qint64 min_key, size_t &vec_i)
{
    Q_ASSERT(static_cast<size_t>(static_cast<int>(num_datapoints)) == num_datapoints);

    // search from v[vec_i] using indexes starting from 0
    vector = &vector[vec_i];

    // load 4 subarray offsets into xveci
    size_t num_one = num_datapoints / 3;

    // tmp_i = vec_i + step;
    __m128i xtmpi = _mm_set_epi32(0, 0, static_cast<int>(num_one), static_cast<int>(num_one*2));

    // sizeof(vector[0]) == 2 * 8 (use scale 8 for gather)
    __m128i xkeys = _mm_i32gather_epi64(reinterpret_cast<const long long *>(vector),
                                        _mm_slli_epi32(xtmpi, 1), 8);
    __m128i xcmpg = _mm_cmpgt_epi64(xkeys, _mm_set1_epi64x(min_key));

    // each 2 bits in mask = (key > min) for each point in xveci
    // return a) the first index where key > min
    //        b) the last index (all keys were < min)
    unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(xcmpg));
    size_t step;
    switch (mask) {
    case 0x0000:
        step = num_one*2;
        vec_i += step + 1;
        num_datapoints -= step + 1;
        break;
    case 0xFFFF:
        num_datapoints = num_one;
        break;
    case 0x00FF:
        vec_i += num_one + 1;
        num_datapoints = num_one;
        break;
    }
}

// reduce array length to 31 bits and use avx
static inline size_t hist_skip_front_avx(const Datapoint *vector, size_t num_datapoints,
                                         qint64 min_key, size_t vec_i)
{
    Q_ASSERT(sizeof(vector[0]) == 16);
    Q_ASSERT(vector != nullptr && num_datapoints > 0);
    Q_ASSERT(vec_i < num_datapoints);

    min_key -= 1;
    size_t tmp_i = 0;
    while (num_datapoints > 1LL<<30) {
        size_t step = 0;
        if (hist_skip_front_single_step(vector, num_datapoints, min_key, vec_i, tmp_i, step)) {
            vec_i = (tmp_i + 1);
            num_datapoints -= (step + 1);
        } else {
            num_datapoints = step;
        }
    }
    while (num_datapoints >= 3) {
        hist_skip_front_avx_step(vector, num_datapoints, min_key, vec_i);
    }
    return vec_i + hist_skip_front_single(&vector[vec_i], num_datapoints, min_key+1, 0);
}
#endif

static inline size_t hist_skip_front(const Datapoint *vector, size_t num_datapoints,
                                     qint64 min_key, bool avx)
{
    size_t vec_i = 0;
#ifdef DIRAGE_HIST_VECTORIZE
    if (avx) {
        vec_i = hist_skip_front_avx(vector, num_datapoints, min_key, vec_i);
    } else {
        vec_i = hist_skip_front_single(vector, num_datapoints, min_key, vec_i);
    }
#else
    vec_i = hist_skip_front_single(vector, num_datapoints, min_key, vec_i);
#endif
    return vec_i;
}


#include <QDebug>
qint64 make_histogram(const Datapoint *vector, size_t num_datapoints, qint64 min_key, qint64 max_key,
                      qint64 *bins, size_t num_bins, bool avx)
{
    Q_ASSERT(vector != nullptr && bins != nullptr && num_bins > 0 && num_datapoints > 0);
    Q_ASSERT(num_bins < 1LL<<16);
    Q_ASSERT(max_key >= min_key);

#ifdef DIRAGE_HIST_VECTORIZE
    static_assert(sizeof(vector[0]) == 16, "AVX version is coded with 16-byte data point");
#endif

    size_t vec_i = 0;
    vec_i = hist_skip_front(vector, num_datapoints, min_key, avx);
    //qInfo() << "avx:" << avx << "vec_i" << vec_i;

    qint64 current_min_key = min_key;
    qint64 biggest_bin_size = 0;
    for (size_t bin_i = 0; bin_i < num_bins; ++bin_i) {
        qint64 remaining_bins = static_cast<qint64>(num_bins - bin_i);
        qint64 bin_max_key = current_min_key + ((max_key - current_min_key) / remaining_bins);
        qint64 bin_size = 0;
        while (vec_i < num_datapoints  && vector[vec_i].key <= bin_max_key) {
            bin_size += vector[vec_i].value;
            vec_i++;
        }
        current_min_key = bin_max_key;
        bins[bin_i] = bin_size;
        if (biggest_bin_size < bin_size) {
            biggest_bin_size = bin_size;
        }
    }
    return biggest_bin_size;
}

} // namespace platform
//#include "immintrin.h"





