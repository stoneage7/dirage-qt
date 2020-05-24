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

AgeHistogram::AgeHistogram(const AgeVector &vector, int numBins,
                           qint64 minTimestamp, qint64 maxTimestamp):
    m_bins(numBins)
{
    m_medianTimestamp = vector.medianTimestamp();

    histogram::Impl *impl = platform::histogramImpl;
    impl->make(vector.datapoints().cbegin(), vector.datapoints().cend(),
               minTimestamp, maxTimestamp, m_bins.begin(), m_bins.end());
    m_totalSize = impl->sumValues(m_bins.cbegin(), m_bins.cend());
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
    return platform::histogramImpl->largestValue(m_bins.cbegin()+fromIndex,
                                                 m_bins.cbegin()+toIndex+1);
}

std::pair<qint64, qint64> AgeHistogram::largestBinAndSum(int fromIndex, int toIndex) const
{
    Q_ASSERT(fromIndex >= 0 && toIndex >= 0 &&
             fromIndex < m_bins.length() && toIndex < m_bins.length());
    return platform::histogramImpl->largestValueAndSum(m_bins.cbegin()+fromIndex,
                                                       m_bins.cbegin()+toIndex+1);
}
