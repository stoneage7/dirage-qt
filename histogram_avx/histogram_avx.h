#ifndef HISTOGRAM_AVX_H
#define HISTOGRAM_AVX_H

#include "histogram.h"

namespace histogram {

class AVX2Impl : public ScalarImpl
{
protected:
    static void lowerBoundStep(Impl::VecIter &begin, int64_t &length, Datapoint::KeyType minKey);
    static void lowerBound(VecIter begin, VecIter end, Datapoint::KeyType minKey);

public:
    virtual void make(VecIter begin, VecIter end,
                      Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
                      BinIter binBegin, BinIter binEnd);
    virtual ~AVX2Impl();
};

} // namespace histogram

#endif // HISTOGRAM_AVX_H
