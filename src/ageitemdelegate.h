#ifndef AGEITEMDELEGATE_H
#define AGEITEMDELEGATE_H

#include <QtCore>
#include <QAbstractItemDelegate>

class AgeItemDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

private:
    int m_numVisibleBins;
    int m_firstVisibleBin;
    qint64 m_largestBinInView;
    bool m_gridLinesToggle;

public:
    AgeItemDelegate(QObject *parent);
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

    int numVisibleBins() const { return m_numVisibleBins; }
    int firstVisibleBin() const { return m_firstVisibleBin; }
    qint64 largestBinInView() const { return m_largestBinInView; }

public slots:
    void setLargestBinInView(qint64 newLargestBin) { m_largestBinInView = newLargestBin; }
    void setNumVisibleBins(int newNum) { m_numVisibleBins = newNum; }
    void setFirstVisibleBin(int newFirst) { m_firstVisibleBin = newFirst; }
    void setGridLinesToggle(int state) { m_gridLinesToggle = (state == Qt::Checked ); }
};

#endif // AGEITEMDELEGATE_H
