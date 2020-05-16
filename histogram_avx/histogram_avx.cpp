
#include "histogram_avx.h"
#include "immintrin.h"

namespace histogram {

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

template <typename I, typename P>
static inline const P *iter_to_pointer_type(const I iter) { return static_cast<const P*>(iter); }

template <>
inline const long long *iter_to_pointer_type(AVX2Impl::VecIter iter)
{
    const Datapoint *tmp = static_cast<const Datapoint*>(iter);
    const long long *rv = reinterpret_cast<const long long*>(tmp);
    return rv;
}

inline void AVX2Impl::lowerBoundStep(VecIter &begin, int64_t &length, Datapoint::KeyType minKey)
{
    const long long* baseAddr = iter_to_pointer_type<VecIter, long long>(begin);

    // load 4 subarray offsets into xveci
    int64_t numOne = length / 3;

    // tmp_i = vec_i + step;
    __m128i xtmpi = _mm_set_epi32(0, 0, static_cast<int>(numOne), static_cast<int>(numOne*2));

    // sizeof(vector[0]) == 2 * 8 (use scale 8 for gather)
    __m128i xkeys = _mm_i32gather_epi64(baseAddr, _mm_slli_epi32(xtmpi, 1), 8);
    __m128i xcmpg = _mm_cmpgt_epi64(xkeys, _mm_set1_epi64x(minKey));

    // each 2 bits in mask = (key > min) for each point in xveci
    // return a) the first index where key > min
    //        b) the last index (all keys were < min)
    unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(xcmpg));
    int64_t step;
    switch (mask) {
    case 0x0000:
        step = numOne*2;
        std::advance(begin, step + 1);
        length -= step + 1;
        break;
    case 0xFFFF:
        length = numOne;
        break;
    case 0x00FF:
        std::advance(begin, numOne + 1);
        length = numOne;
        break;
    }
}

void AVX2Impl::lowerBound(VecIter begin, VecIter end, Datapoint::KeyType minKey)
{
    // reduce array length to 30 bits and use avx
    int64_t length = end - begin;
    while (length > 1LL<<30) {
        ScalarImpl::lowerBoundStep(begin, length, minKey);
    }
    while (length > 3) {
        AVX2Impl::lowerBoundStep(begin, length, minKey);
    }
}

void AVX2Impl::make(VecIter begin, VecIter end,
                    Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
                    BinIter binBegin, BinIter binEnd)
{
    AVX2Impl::lowerBound(begin, end, minKey);
    ScalarImpl::makeImpl(begin, end, minKey, maxKey, binBegin, binEnd);
}

AVX2Impl::~AVX2Impl()
{
}

} // namespace histogram
