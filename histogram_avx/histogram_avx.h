#ifndef HISTOGRAM_AVX_H
#define HISTOGRAM_AVX_H

#include "histogram.h"

namespace histogram {

class AVX2Impl : public ScalarImpl
{
protected:
    static void
    lowerBoundStep(Impl::VecIter &begin, int64_t &length, Datapoint::KeyType minKey);

    static void
    lowerBound(VecIter begin, VecIter end, Datapoint::KeyType minKey);

    static Datapoint::ValueType accumulateBin(VecIter &begin, VecIter end,
                                              Datapoint::KeyType binMaxKey);
    static void makeImpl(VecIter begin, VecIter end,
                          Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
                          BinIter binBegin, BinIter binEnd);

public:
    virtual void make(VecIter begin, VecIter end,
                      Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
                      BinIter binBegin, BinIter binEnd) override;

    virtual Datapoint::ValueType
    sumValues(BinConstIter from, BinConstIter to) override;

    virtual std::pair<Datapoint::ValueType, Datapoint::ValueType>
    largestValueAndSum(BinConstIter from, BinConstIter to) override;

    virtual ~AVX2Impl() override;
};

} // namespace histogram

#endif // HISTOGRAM_AVX_H
