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

class Vector {
public:
    typedef std::vector<Datapoint> Container;
    typedef Container::const_iterator ContainerIter;

protected:
    Container m_vec;
    bool m_isFinalized;
    static void lowerBoundStep(ContainerIter &begin, size_t &length, Datapoint::KeyType minKey);

public:
    Vector(): m_isFinalized(false) { }
    void finalize();
    inline Container::const_iterator begin() const { return m_vec.cbegin(); }
    inline Container::const_iterator end() const { return m_vec.end(); }
    inline Container::size_type size() { return m_vec.size(); }
    ContainerIter lowerBound(Datapoint::KeyType minKey) const;
};

class Histogram {
protected:
    typedef std::vector<Datapoint::KeyType> BinsContainer;
    //typedef QVector<Datapoint>::const_iterator VecIter;

    static Datapoint::ValueType accumulateBin(Vector::ContainerIter &iter,
                                              Vector::ContainerIter end,
                                              Datapoint::KeyType binMaxKey);

    BinsContainer m_bins;

public:
    Histogram(const Vector &vector, Datapoint::KeyType minKey, Datapoint::KeyType maxKey,
              uint32_t numBins);
};

}

#endif // HISTOGRAM_H
