#include "agehistogram.h"
#include "platform.h"
#include <algorithm>

void AgeVector::finalize()
{
    std::sort(m_vec.begin(), m_vec.end(),
              [] (const AgeDatapoint& a, const AgeDatapoint &b) -> bool { return a.timestamp < b.timestamp; }
    );

    qint64 a = 0;
    for (auto i = m_vec.cbegin(); i != m_vec.cend(); ++i) {
        a += (*i).size;
        if (a >= m_totalSize / 2) {
            m_medianTimestamp.set((*i).timestamp);
            break;
        }
    }
}

AgeHistogram::AgeHistogram(const AgeVector &vector, int num_buckets,
                           qint64 min_timestamp, qint64 max_timestamp):
    m_bins(num_buckets),
    m_maxBucketSize(0)
{
    static_assert((sizeof(AgeDatapoint) == sizeof(platform::Datapoint)) &&
                  (offsetof(AgeDatapoint, timestamp) == offsetof(platform::Datapoint, key)) &&
                  (offsetof(AgeDatapoint, size) == offsetof(platform::Datapoint, value)),
                  "AgeDatapoint <-> platform::Datapoint layout mismatched");
    m_minTimestamp.set(min_timestamp);
    m_maxTimestamp.set(max_timestamp);
    m_medianTimestamp = vector.medianTimestamp();

    platform::make_histogram(reinterpret_cast<const platform::Datapoint *>(vector.datapoints().begin()),
                             static_cast<size_t>(vector.datapoints().length()),
                             min_timestamp, max_timestamp, m_bins.data(),
                             static_cast<size_t>(m_bins.length()), true);
}

QString
AgeHistogram::toString() const
{
    QStringList s;
    for (auto i = m_bins.begin(); i != m_bins.end(); ++i) {
        s.append(QString::number(*i));
    }
    return s.join(", ");
}
