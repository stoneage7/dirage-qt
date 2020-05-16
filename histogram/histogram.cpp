
#include "histogram.h"
//#include <iterator>

namespace histogram {

inline void
Vector::lowerBoundStep(Vector::ContainerIter &begin, size_t &length, Datapoint::KeyType minKey)
{
    size_t step = length >> 1;
    Vector::ContainerIter tmp = begin;
    std::advance(tmp, step);
    if (tmp->key < minKey) {
        begin = tmp + 1;
        length -= step + 1;
    } else {
        length -= step;
    }
}

void Vector::finalize()
{
    std::sort(m_vec.begin(), m_vec.end(),
              [] (const Datapoint &a, const Datapoint &b) { return a.key < b.key; });
    m_isFinalized = true;
}

Vector::ContainerIter Vector::lowerBound(Datapoint::KeyType minKey) const
{
    size_t length = m_vec.size();
    ContainerIter begin = m_vec.cbegin();
    while (length > 0) {
        Vector::lowerBoundStep(begin, length, minKey);
    }
    return begin;
}

Histogram::Histogram(const Vector &vector, Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
                     uint32_t numBins)
    :m_bins(numBins, 0)
{
    Q_ASSERT(maxKey >= minKey);
    Vector::ContainerIter begin = vector.lowerBound(minKey);
    Datapoint::KeyType currentMinKey = minKey;
    for (size_t bin_i = 0; bin_i < numBins; ++bin_i) {
        size_t remainingBins = numBins - bin_i;
        Datapoint::KeyType binMaxKey =
                currentMinKey + ((maxKey - currentMinKey)  / static_cast<int64_t>(remainingBins));
        Datapoint::ValueType binSize = 0;
        while (begin < vector.end() && (*begin).value <= binMaxKey) {
            binSize += (*begin).value;
            begin++;
        }
    }

}

} // namespace histogram
