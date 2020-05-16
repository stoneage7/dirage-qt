
#include <QPainter>
#include "ageitemdelegate.h"

const int DEFAULT_CHART_WIDTH = 500;
const int LARGEST_BUCKET_SIZE_PX = 100;
const int DEFAULT_NUM_VISIBLE_BINS = 50;

AgeItemDelegate::AgeItemDelegate(QObject *parent)
   :QAbstractItemDelegate(parent)
   ,m_numVisibleBins(DEFAULT_NUM_VISIBLE_BINS)
   ,m_firstVisibleBin(0)
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
    int remainingLength = option.rect.width();
    int nextX = 0;
    int maxHeight = option.rect.height();
    for (int i = 0; i < m_numVisibleBins; i++) {
        int j = i + m_firstVisibleBin;
        if (j >= bins.length()) {
            break;
        }
        int remainingBins = m_numVisibleBins;
        int barWidth = remainingLength / remainingBins;
        int bar_height = 0;
        if (m_largestBinInView > 0) {
            bar_height = static_cast<int>(bins.at(j) * maxHeight / m_largestBinInView);
            if (bins.at(j) > 0 && bar_height == 0) {
                bar_height = 1;
            }
        }
        painter->fillRect(nextX, (option.rect.height() - bar_height) / 2,
                          barWidth, bar_height, QBrush(Qt::blue));
        nextX += barWidth;
        remainingLength -= barWidth;
    }
    painter->restore();
}

QSize AgeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(DEFAULT_CHART_WIDTH, LARGEST_BUCKET_SIZE_PX);
}


void AgeItemDelegate::updateLargestBinInView(const AgeModel *model)
{
    if (model != nullptr) {
        m_largestBinInView = model->largestBinSize(m_firstVisibleBin,
                                                   m_firstVisibleBin + m_numVisibleBins - 1);
    }
}

void AgeItemDelegate::setNumVisibleBins(int newNum, const AgeModel *model)
{
    m_numVisibleBins = newNum;
    this->updateLargestBinInView(model);
}

void AgeItemDelegate::setFirstVisibleBin(int newFirst, const AgeModel *model)
{
    m_firstVisibleBin = newFirst;
    this->updateLargestBinInView(model);
}
