
#include "histogram.h"

#include <QtDebug>

namespace histogram {

std::pair<Datapoint::KeyType, Datapoint::KeyType>
Impl::binRange(Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
                     int numBins, int binIndex)
{
    Datapoint::KeyType resultMin, resultMax;
    const float binRange = static_cast<float>(maxKey - minKey) / static_cast<float>(numBins);
    if (binIndex == numBins - 1) {
        resultMin = minKey + static_cast<Datapoint::KeyType>((binIndex) * binRange);
        resultMax = maxKey;
    } else {
        if (binIndex == 0) {
            resultMin = minKey;
        } else {
            resultMin = minKey + static_cast<Datapoint::KeyType>((binIndex) * binRange);
        }
        resultMax = minKey + static_cast<Datapoint::KeyType>((binIndex+1) * binRange);
    }
    return std::pair<Datapoint::KeyType, Datapoint::KeyType>(resultMin, resultMax);
}

Impl::~Impl() {  }

void ScalarImpl::lowerBoundStep(VecIter &begin, int64_t &length, Datapoint::KeyType minKey)
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

void ScalarImpl::lowerBound(VecIter begin, VecIter end, Datapoint::KeyType minKey)
{
    int64_t length = std::distance(begin, end);
    while (length > 0) {
        ScalarImpl::lowerBoundStep(begin, length, minKey);
    }
}

Datapoint::ValueType ScalarImpl::accumulateBin(VecIter &begin, ScalarImpl::VecIter end,
                                               Datapoint::KeyType binMaxKey)
{
    Datapoint::ValueType binSize = 0;
    while (begin < end && (*begin).key <= binMaxKey) {
        binSize += (*begin).value;
        begin++;
    }
    return binSize;
}

void ScalarImpl::makeImpl(VecIter begin, VecIter end,
                          Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
                          BinIter binBegin, BinIter binEnd)
{
    auto numBins = binEnd - binBegin;
    const float binRange = static_cast<float>(maxKey - minKey) / static_cast<float>(numBins);
    for (int bin_i = 0; bin_i < numBins; ++bin_i) {
        Datapoint::KeyType binMaxKey;
        if (bin_i == numBins - 1) {
            binMaxKey = maxKey;
        } else {
            binMaxKey = minKey + static_cast<Datapoint::KeyType>((bin_i+1) * binRange);
        }
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

Datapoint::ValueType ScalarImpl::sumValues(BinConstIter from, BinConstIter to)
{
    Datapoint::ValueType sum = 0;
    for (; from != to; from++) {
        sum += (*from);
    }
    return sum;
}

std::pair<Datapoint::ValueType, Datapoint::ValueType>
ScalarImpl::largestValueAndSum(BinConstIter from, BinConstIter to)
{
    Datapoint::ValueType max = 0;
    Datapoint::ValueType sum = 0;
    for (; from != to; from++) {
        if (max < (*from)) {
            max = (*from);
        }
        sum += (*from);
    }
    return std::pair<qint64, qint64>(max, sum);
}

ScalarImpl::~ScalarImpl() { }

} // namespace histogram
