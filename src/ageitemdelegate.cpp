#include "ageitemdelegate.h"
#include "agemodel.h"
#include <QPainter>
#include <QDateTime>

const int DEFAULT_CHART_WIDTH = 500;
const int LARGEST_BUCKET_SIZE_PX = 100;

AgeItemDelegate::AgeItemDelegate(QObject *parent):
   QAbstractItemDelegate(parent)
{

}

int AgeItemDelegate::calculateRowHeight(qint64 maxGlobalBucketSize, qint64 maxBucketSize)
{
    if (maxGlobalBucketSize == 0) {
        return 0;
    } else {
        return static_cast<int> (maxBucketSize * LARGEST_BUCKET_SIZE_PX / maxGlobalBucketSize);
    }
}

void
AgeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    AgeRenderData value = index.data().value<AgeRenderData>();
    const QVector<qint64> &bins = value.row.histogram.bins();

    painter->save();
    painter->translate(option.rect.x(), option.rect.y());
    int remaining_length = option.rect.width();
    int next_x = 0;
    int max_height = option.rect.height();
    for (int i = 0; i < bins.length(); i++) {
        int remaining_bins = bins.length() - i;
        int bar_width = remaining_length / remaining_bins;
        int bar_height = 0;
        if (value.global.maxBinSize > 0) {
            bar_height = static_cast<int>(bins.at(i) * max_height / value.global.maxBinSize);
            if (bins.at(i) > 0 && bar_height == 0) {
                bar_height = 1;
            }
        }
        painter->fillRect(next_x, (option.rect.height() - bar_height) / 2,
                          bar_width, bar_height, QBrush(Qt::blue));
        next_x += bar_width;
        remaining_length -= bar_width;
    }
    painter->restore();
}

QSize AgeItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(DEFAULT_CHART_WIDTH, LARGEST_BUCKET_SIZE_PX);
}
