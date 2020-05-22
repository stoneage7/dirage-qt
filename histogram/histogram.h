#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QtCore>

namespace histogram {

struct Datapoint {
    typedef qint64 KeyType;
    typedef qint64 ValueType;
    KeyType key;
    ValueType value;
};

class Impl {
public:
    typedef QVector<Datapoint>::const_iterator VecIter;
    typedef QVector<Datapoint::ValueType>::iterator BinIter;

    virtual void
    make(VecIter begin, VecIter end, Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
         BinIter binBegin, BinIter binEnd) = 0;

    // this function is only for display purpose
    std::pair<Datapoint::KeyType, Datapoint::KeyType>
    binRange(Datapoint::KeyType minKey, Datapoint::KeyType maxkey, int numBins, int binIndex);

    virtual ~Impl();
};

class ScalarImpl : public Impl {
protected:
    static void lowerBoundStep(Impl::VecIter &begin, int64_t &length, Datapoint::KeyType minKey);
    static void lowerBound(VecIter begin, VecIter end, Datapoint::KeyType minKey);

    static Datapoint::ValueType
    accumulateBin(VecIter &begin, VecIter end, Datapoint::KeyType binMaxKey);

    static void
    makeImpl(VecIter begin, VecIter end, Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
             BinIter binBegin, BinIter binEnd);

public:
    virtual void
    make(VecIter begin, VecIter end, Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
         BinIter binBegin, BinIter binEnd) override;

    virtual ~ScalarImpl() override;

protected:
};

}

#endif // HISTOGRAM_H
