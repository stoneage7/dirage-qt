
#include "histogram.h"

namespace histogram {

Impl::~Impl() {  }

void
ScalarImpl::lowerBoundStep(VecIter &begin, int64_t &length, Datapoint::KeyType minKey)
{
    auto step = length >> 1;
    auto tmp = begin;
    std::advance(tmp, step);
    if (tmp->key < minKey) {
        begin = tmp + 1;
        length -= step + 1;
    } else {
        length = step;
    }
}

void
ScalarImpl::lowerBound(VecIter begin, VecIter end, Datapoint::KeyType minKey)
{
    int64_t length = std::distance(begin, end);
    while (length > 0) {
        ScalarImpl::lowerBoundStep(begin, length, minKey);
    }
}

Datapoint::ValueType
ScalarImpl::accumulateBin(ScalarImpl::VecIter begin, ScalarImpl::VecIter end,
                          Datapoint::KeyType binMaxKey)
{
    Datapoint::ValueType binSize = 0;
    while (begin < end && (*begin).value <= binMaxKey) {
        binSize += (*begin).value;
        begin++;
    }
    return binSize;
}

void
ScalarImpl::makeImpl(VecIter begin, VecIter end,
               Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
               BinIter binBegin, BinIter binEnd)
{
    Datapoint::KeyType currentMinKey = minKey;
    auto numBins = binEnd - binBegin;
    for (int bin_i = 0; bin_i < numBins; ++bin_i) {
        long remainingBins = numBins - bin_i;
        Datapoint::KeyType binMaxKey =
                currentMinKey + ((maxKey - currentMinKey)  / (remainingBins));
        *(binBegin + bin_i) = ScalarImpl::accumulateBin(begin, end, binMaxKey);
    }
}

void ScalarImpl::make(ScalarImpl::VecIter begin, ScalarImpl::VecIter end,
                      Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
                      ScalarImpl::BinIter binBegin, ScalarImpl::BinIter binEnd)
{
    Q_ASSERT(maxKey >= minKey);
    ScalarImpl::lowerBound(begin, end, minKey);
    ScalarImpl::makeImpl(begin, end, minKey, maxKey, binBegin, binEnd);
}

ScalarImpl::~ScalarImpl() { }


} // namespace histogram
