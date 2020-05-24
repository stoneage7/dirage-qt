
#include "histogram_avx.h"
#include "immintrin.h"
#include <type_traits>

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

template <typename I, typename P>
static inline const P *iter_to_pointer_type(const I iter) { return static_cast<const P*>(iter); }

template <>
inline const long long *iter_to_pointer_type(AVX2Impl::VecIter iter)
{
    const Datapoint *tmp = static_cast<const Datapoint*>(iter);
    const long long *rv = reinterpret_cast<const long long*>(tmp);
    return rv;
}

template <>
inline const __m256i_u *iter_to_pointer_type(AVX2Impl::VecIter iter)
{
    const Datapoint *tmp = static_cast<const Datapoint*>(iter);
    const __m256i_u *rv = reinterpret_cast<const __m256i_u*>(tmp);
    return rv;
}

template <>
inline const __m256i_u *iter_to_pointer_type(AVX2Impl::BinConstIter iter)
{
    const __m256i_u *rv = reinterpret_cast<const __m256i_u*>(iter);
    return rv;
}

inline void AVX2Impl::lowerBoundStep(VecIter &begin, int64_t &length, Datapoint::KeyType minKey)
{
    const long long* baseAddr = iter_to_pointer_type<VecIter, long long>(begin);
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

inline void AVX2Impl::lowerBound(VecIter begin, VecIter end, Datapoint::KeyType minKey)
{
    // reduce array length to 30 bits and use avx
    int64_t length = end - begin;
    while (length > 1LL<<30) {
        ScalarImpl::lowerBoundStep(begin, length, minKey);
    }
    while (length > 3) {
        AVX2Impl::lowerBoundStep(begin, length, minKey);
    }
    while (length > 1) {
        ScalarImpl::lowerBoundStep(begin, length, minKey);
    }
}

inline Datapoint::ValueType AVX2Impl::accumulateBin(VecIter &begin,  VecIter end,
                                                    Datapoint::KeyType binMaxKey)
{
    static_assert(sizeof(*begin) == 16 && sizeof(begin->key) == 8 && sizeof(begin->value) == 8 &&
                  offsetof(Datapoint, key) == 0 && offsetof(Datapoint, value) == 8,
                  "AVX2Impl::accumulateBin() - only { 64bit int key, 64bit int value }");

    __m256i acc = _mm256_set1_epi64x(0); // accumulators
    const VecIter oldBegin = begin;
    const __m256i maxKeys = _mm256_set1_epi64x(binMaxKey);
    for (; end - begin >= 4; begin += 4) {
        const __m256i_u *reg1Ptr = iter_to_pointer_type<VecIter, __m256i_u>(begin);
        const __m256i_u *reg2Ptr = iter_to_pointer_type<VecIter, __m256i_u>(begin+2);
        const __m256i reg1 = _mm256_loadu_si256(reg1Ptr);
        const __m256i reg2 = _mm256_loadu_si256(reg2Ptr);

        // reg1=[k1 v1 k2 v2] reg2=[k3 v3 k4 v4]
        // unpacklo / unpackhi
        // reg3=[k1 k3 k2 k4] reg4=[v1 v3 v2 v4]
        const __m256i keys = _mm256_unpacklo_epi64(reg1, reg2);
        const __m256i vals = _mm256_unpackhi_epi64(reg1, reg2);

        __m256i testGt = _mm256_cmpgt_epi64(keys, maxKeys);

        // add vals where key<=maxKey
        acc = _mm256_add_epi64(acc, _mm256_andnot_si256(testGt, vals));

        if (!_mm256_testz_si256(testGt, testGt)) {
            // at least one key>maxKey
            // permute 11011000 = 0xD8
            // reg1=[k1 k2 k3 k4] <- across lanes k2 k3
            testGt = _mm256_permute4x64_epi64(testGt, 0x27);
            const unsigned mask = static_cast<unsigned>(_mm256_movemask_epi8(testGt));
            if (mask != 0) {
#ifdef __GNUC__
                const int clz = __builtin_clz(mask);
#else
# error supply clz function for non-gcc
#endif
                std::advance(begin, clz >> 3);
                break;
            } else {
                // all keys>maxKey
                std::advance(begin, 4);
                break;
            }
        }
    }

    Datapoint::ValueType binSize = 0;
    if (begin > oldBegin) {
        __m256i_u unload;
        _mm256_storeu_si256(&unload, acc);
        for (int i = 0; i < 4; i++) {
            binSize += unload[i];
        }
    }
    if (begin < end) {
        binSize += ScalarImpl::accumulateBin(begin, end, binMaxKey);
    }
    return binSize;
}

inline void AVX2Impl::makeImpl(VecIter begin, VecIter end,
                               Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
                               BinIter binBegin, BinIter binEnd)
{
    auto numBins = binEnd - binBegin;
    const __m256 binRange = _mm256_set1_ps(static_cast<float>(maxKey - minKey)
                                           / static_cast<float>(numBins));
    __m256 binIndex = _mm256_setr_ps(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0);
    for (int binI = 0; binI < numBins; binI += 8) {
        __m256 part = _mm256_mul_ps(binIndex, binRange);
        float partU[8];
        _mm256_storeu_ps(partU, part);
        for (int binJ = 0; binJ < 8 && binI + binJ < numBins; binJ++) {
            Datapoint::KeyType binMaxKey;
            if (binI + binJ == numBins - 1) {
                binMaxKey = maxKey;
            } else {
                binMaxKey = minKey + static_cast<Datapoint::KeyType>(partU[binJ]);
            }
            *(binBegin + binI + binJ) = AVX2Impl::accumulateBin(begin, end, binMaxKey);
        }
        binIndex = _mm256_add_ps(binIndex, _mm256_set1_ps(8.0));
    }
}

void AVX2Impl::make(VecIter begin, VecIter end,
                    Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
                    BinIter binBegin, BinIter binEnd)
{
    AVX2Impl::lowerBound(begin, end, minKey);
    AVX2Impl::makeImpl(begin, end, minKey, maxKey, binBegin, binEnd);
}

Datapoint::ValueType AVX2Impl::largestValue(BinConstIter from, BinConstIter to)
{
    static_assert(sizeof(*from) == 8 && std::is_pointer<BinIter>::value,
                  "AVX2Impl::largestValue() - 64bit ints only");
    __m256i maxBins = _mm256_set1_epi64x(0);
    while (to - from > 4) {
        const __m256i_u *binsPtr = iter_to_pointer_type<BinConstIter, __m256i_u>(from);
        const __m256i bins = _mm256_loadu_si256(binsPtr);
        const __m256i cmpgt = _mm256_cmpgt_epi64(bins, maxBins);
        maxBins = _mm256_or_si256(_mm256_andnot_si256(cmpgt, maxBins),
                                  _mm256_and_si256(bins, cmpgt));
        std::advance(from, 4);
    }

    Datapoint::ValueType max = 0;
    __m256i_u maxU;
    _mm256_storeu_si256(&maxU, maxBins);
    for (int i = 0; i < 4; i++) {
        if (max < maxBins[i]) {
            max = maxBins[i];
        }
    }
    if (from < to) {
       Datapoint::ValueType restMax = ScalarImpl::largestValue(from, to);
       if (max < restMax) {
           max = restMax;
       }
    }
    return max;
}

Datapoint::ValueType AVX2Impl::sumValues(BinConstIter from, BinConstIter to)
{
    static_assert(sizeof(*from) == 8 && std::is_pointer<BinIter>::value,
                  "AVX2Impl::sumValues() - 64bit ints only");
    __m256i acc = _mm256_set1_epi64x(0);
    while (to - from > 4) {
        const __m256i_u *binsPtr = iter_to_pointer_type<BinConstIter, __m256i_u>(from);
        acc = _mm256_add_epi64(acc, _mm256_loadu_si256(binsPtr));
        std::advance(from, 4);
    }

    __m256i_u accU;
    _mm256_storeu_si256(&accU, acc);
    Datapoint::ValueType sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += acc[i];
    }

    if (from < to) {
        sum += ScalarImpl::sumValues(from, to);
    }

    return sum;
}

AVX2Impl::~AVX2Impl()
{
}

} // namespace histogram
