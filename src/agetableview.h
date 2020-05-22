#ifndef AGETABLEVIEW_H
#define AGETABLEVIEW_H

#include <QtCore>
#include <QTableView>
#include <QLabel>
#include "ageitemdelegate.h"
#include "agemodel.h"

class AgeTableView : public QTableView
{
    Q_OBJECT

private:
    AgeItemDelegate m_histogramDelegate;
    qint64 largestBinSizeInView(AgeModel *model);
    void updateLabels(AgeModel *model);

public:
    AgeTableView(QWidget *parent = nullptr);
    virtual ~AgeTableView() { }
    void connectScollBar(QScrollBar *sb);
    void connectZoomSlider(QSlider *zoomSlider);
    void connectLabels(QLabel *minTsLabel, QLabel *maxTsLabel);

signals:
    void setScrollMax(int newMax);
    void setZoomRange(int newMin, int newMax);
    void setMinLabel(QString newText);
    void setMaxLabel(QString newText);

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
