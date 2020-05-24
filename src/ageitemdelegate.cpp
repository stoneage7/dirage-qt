
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
   ,m_gridLinesToggle(false)
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

    const int maxHeight = option.rect.height();

    // paint center line
    painter->setPen(Qt::darkGray);
    painter->drawLine(0, maxHeight / 2, option.rect.width(), maxHeight / 2);

    // paint horizontal gridlines
    if (m_gridLinesToggle) {
        const qint64 tmpGig = 1024LL*1024LL*1024LL;
        const qint64 tmpMeg = 1024LL*1024LL;
        const qreal tmpLargest = static_cast<qreal>(m_largestBinInView);
        const struct { qint64 unitSize; int h; int l; } gridColors[] =
        {
        { 100LL*tmpGig, 0, 200 }, // gigs = red, megs = green
        { 10LL*tmpGig, 0, 150 },
        { 1LL*tmpGig, 0, 100 },
        { 100LL*tmpMeg, 120, 200 },
        { 10LL*tmpMeg, 120, 150 },
        { 1LL*tmpMeg, 120, 100 },
        { 0, 0, 0 }
    };
        int c = 0;
        while (gridColors[c].unitSize > 0) {
            if (gridColors[c].unitSize <= m_largestBinInView / 1.5) {
                break;
            }
            c++;
        }
        if (gridColors[c].unitSize != 0) {
            const qreal fraction = tmpLargest / static_cast<qreal>(gridColors[c].unitSize);
            painter->setPen(QColor::fromHsl(gridColors[c].h, 80, gridColors[c].l));
            for (qreal j = 1; j < static_cast<qreal>(fraction); j += 1.0) {
                const int offsetY = static_cast<int>(option.rect.height() / fraction * j / 2);
                const int center = option.rect.height() / 2;
                painter->drawLine(0, center-offsetY, option.rect.width(), center-offsetY);
                painter->drawLine(0, center+offsetY, option.rect.width(), center+offsetY);
            }
        }
    }

    // paint chart
    int remainingLength = option.rect.width();
    int nextX = 0;
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
