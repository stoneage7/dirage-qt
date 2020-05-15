#ifndef AGEITEMDELEGATE_H
#define AGEITEMDELEGATE_H

#include <QtCore>
#include <QAbstractItemDelegate>
#include "agemodel.h"

class AgeItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

private:
    int m_numVisibleBins;
    int m_firstVisibleBin;
    qint64 m_largestBinInView;

public:
    AgeItemDelegate(QObject *parent);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    int numVisibleBins() const { return m_numVisibleBins; }
    int firstVisibleBin() const { return m_firstVisibleBin; }

public slots:
    void setNumVisibleBins(int newNum);
    void setFirstVisibleBin(int newFirst, const AgeModel *model);
};

#endif // AGEITEMDELEGATE_H
