#ifndef AGEHISTOGRAM_H
#define AGEHISTOGRAM_H

#include <QtCore>

struct AgeDatapoint
{
    qint64 timestamp;
    qint64 size;
    AgeDatapoint(): timestamp(0), size(0) { }
};
Q_DECLARE_METATYPE(AgeDatapoint);

// optional timestamp
class TimestampOption {
private:
    bool m_valid;
    qint64 m_ts;

public:
    TimestampOption(): m_valid(false), m_ts(0) { }
    bool isValid() const { return m_valid == true; }
    void set(qint64 ts) { m_valid = true; m_ts = ts; }
    void clear() { m_valid = false; m_ts = 0; }
    qint64 get() const { Q_ASSERT(m_valid == true); return m_ts; }
};

class AgeVector
{
    typedef QVector<AgeDatapoint> DatapointContainer;

private:
    DatapointContainer m_vec;
    qint64 m_totalSize;
    TimestampOption m_medianTimestamp;

public:
    AgeVector(): m_totalSize(0) { }
    void append(const AgeDatapoint &point) { m_vec.append(point); m_totalSize += point.size; }
    void finalize();
    bool isFinalized() const { return m_medianTimestamp.isValid(); }
    qint64 totalSize() const { return m_totalSize; }
    const DatapointContainer &datapoints() const { return m_vec; }
    qint64 minTimestamp() const { return m_vec.at(0).timestamp; }
    qint64 maxTimestamp() const { return m_vec.at(m_vec.length()-1).timestamp; }
    TimestampOption medianTimestamp() const { return m_medianTimestamp; }
};
Q_DECLARE_METATYPE(AgeVector);

struct AgeHistogram
{
private:
    QVector<qint64> m_bins;
    TimestampOption m_medianTimestamp;

public:
    AgeHistogram() = default;
    AgeHistogram(const AgeHistogram &other) = default;
    AgeHistogram(const AgeVector &vector, int num_buckets, qint64 min_timestamp, qint64 max_timestamp);
    QString toString() const;
    const QVector<qint64> &bins() const { return m_bins; }
    TimestampOption medianTimestamp() const { return m_medianTimestamp; }
    qint64 largestBinSize() const;
    qint64 largestBinSize(int fromIndex, int toIndex) const;
};
Q_DECLARE_METATYPE(AgeHistogram);

#endif // AGEHISTOGRAM_H