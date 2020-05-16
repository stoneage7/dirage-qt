#ifndef AGETABLEVIEW_H
#define AGETABLEVIEW_H

#include <QtCore>
#include <QTableView>
#include "ageitemdelegate.h"

class AgeTableView : public QTableView
{
    Q_OBJECT

private:
    AgeItemDelegate m_histogramDelegate;

public:
    AgeTableView(QWidget *parent = nullptr);
    virtual ~AgeTableView() { }
    void connectScollBar(QScrollBar *sb);
    void connectZoomSlider(QAbstractSlider *zoomSlider);

signals:
    void setScrollMax(int newMax);
    void setZoomRange(int newMin, int newMax);

public slots:
    void numBinsChanged(int newNumBins);
    void histogramScroll(int newScrollValue);
    void histogramZoom(int newZoomValue);

    // QAbstractItemView interface
public:
    virtual void setModel(QAbstractItemModel *model);

    // QAbstractItemView interface
protected slots:
    virtual void rowsInserted(const QModelIndex &parent, int start, int end);
};

#endif // AGETABLEVIEW_H
