#ifndef AGEITEMDELEGATE_H
#define AGEITEMDELEGATE_H

#include <QAbstractItemDelegate>
#include <QStaticText>

class AgeItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    AgeItemDelegate(QObject *parent);
    static int calculateRowHeight(qint64 maxGlobalBucketSize, qint64 maxBucketSize);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // AGEITEMDELEGATE_H
