#include "agehistogram.h"
#include "platform.h"
#include "histogram.h"
#include <algorithm>

void AgeVector::finalize()
{
    std::sort(m_vec.begin(), m_vec.end(),
              [] (const histogram::Datapoint& a, const histogram::Datapoint &b) -> bool {  return a.key < b.key; }
    );

    qint64 a = 0;
    for (auto i = m_vec.cbegin(); i != m_vec.cend(); ++i) {
        a += (*i).value;
        if (a >= m_totalSize / 2) {
            m_medianTimestamp.set((*i).key);
            break;
        }
    }
}

AgeHistogram::AgeHistogram(const AgeVector &vector, int num_buckets,
                           qint64 min_timestamp, qint64 max_timestamp):
    m_bins(num_buckets)
{
    m_medianTimestamp = vector.medianTimestamp();

    histogram::Impl *impl = platform::histogramImpl;
    impl->make(vector.datapoints().cbegin(), vector.datapoints().cend(),
               min_timestamp, max_timestamp, m_bins.begin(), m_bins.end());

   // platform::make_histogram(reinterpret_cast<const platform::Datapoint *>(vector.datapoints().begin()),
   //                          static_cast<size_t>(vector.datapoints().length()),
   //                          min_timestamp, max_timestamp, m_bins.data(),
   //                          static_cast<size_t>(m_bins.length()), true);
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

qint64 AgeHistogram::largestBinSize() const
{
    return this->largestBinSize(0, m_bins.length()-1);
}

qint64 AgeHistogram::largestBinSize(int fromIndex, int toIndex) const
{
    Q_ASSERT(fromIndex >= 0 && toIndex >= 0 &&
             fromIndex < m_bins.length() && toIndex < m_bins.length());
    qint64 largestBin = 0;
    for (int i = fromIndex; i <= toIndex; ++i) {
        if (largestBin < m_bins.at(i)) {
            largestBin = m_bins.at(i);
        }
    }
    return largestBin;
}
