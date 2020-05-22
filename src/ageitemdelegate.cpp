
#include <QPainter>
#include "platform.h"
#include "ageitemdelegate.h"
#include "agemodel.h"

const int DEFAULT_CHART_WIDTH = 500;
const int LARGEST_BIN_HEIGHT_PX = 100;
const int DEFAULT_NUM_VISIBLE_BINS = 50;

AgeItemDelegate::AgeItemDelegate(QObject *parent)
   :QAbstractItemDelegate(parent)
   ,m_numVisibleBins(DEFAULT_NUM_VISIBLE_BINS)
   ,m_firstVisibleBin(0)
   ,m_largestBinInView(0)
{

}

void
AgeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    AgeModelRow value = index.data().value<AgeModelRow>();
    const QVector<qint64> &bins = value.histogram.bins();
    if (bins.length() == 0) {
        return;
    }
    painter->save();
    painter->translate(option.rect.x(), option.rect.y());

    // paint chart
    int remainingLength = option.rect.width();
    int nextX = 0;
    int maxHeight = option.rect.height();
    for (int i = 0; i < m_numVisibleBins; i++) {
        int j = i + m_firstVisibleBin;
        if (j >= bins.length()) {
            break;
        }
        int remainingBins = m_numVisibleBins - i;
        int barWidth = remainingLength / remainingBins;
        int barHeight = 0;
        if (m_largestBinInView > 0) {
            barHeight = static_cast<int>(bins.at(j) * maxHeight / m_largestBinInView);
            if (bins.at(j) > 0 && barHeight == 0) {
                barHeight = 1;
            }
        }
        painter->fillRect(nextX, (option.rect.height() - barHeight) / 2,
                          barWidth, barHeight, QBrush(Qt::blue));
        nextX += barWidth;
        remainingLength -= barWidth;
    }

    // paint median timestamp
    const AgeModel *am = qobject_cast<const AgeModel*>(index.model());
    const qint64 minTimestamp = am->binRange(m_firstVisibleBin).first;
    const qint64 maxTimestamp = am->binRange(m_firstVisibleBin + m_numVisibleBins - 1).second;
    const TimestampOption medianTs = value.histogram.medianTimestamp();
    if (medianTs.isValid() && medianTs.get() >= minTimestamp && medianTs.get() <= maxTimestamp) {
        int medianX = static_cast<int>(
                    static_cast<qreal>(medianTs.get() - minTimestamp)
                    / static_cast<qreal>(maxTimestamp - minTimestamp)
                    * static_cast<qreal>(option.rect.width()));
        painter->setPen(QColor(Qt::black));
        painter->drawLine(medianX, 0, medianX, maxHeight);
    }



    painter->restore();
}

QSize AgeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(DEFAULT_CHART_WIDTH, LARGEST_BIN_HEIGHT_PX);
}
