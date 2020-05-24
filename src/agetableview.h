#ifndef AGETABLEVIEW_H
#define AGETABLEVIEW_H

#include <QtCore>
#include <QTableView>
#include <QLabel>
#include <QStyledItemDelegate>
#include <QCheckBox>
#include "ageitemdelegate.h"
#include "agemodel.h"

class ByteSizeDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    ByteSizeDelegate() { }
    virtual ~ByteSizeDelegate() { }
    virtual QString displayText(const QVariant &value, const QLocale &locale) const;
    static QString byteSizetoString(qint64 bytes);
};

class AgeTableView : public QTableView
{
    Q_OBJECT

private:
    AgeItemDelegate m_histogramDelegate;
    ByteSizeDelegate m_byteSizeDelegate;
    qint64 m_sumValuesInView;
    void updateLargestAndSum(AgeModel *model);
    void updateLabels(AgeModel *model);

protected:
    virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

public:
    AgeTableView(QWidget *parent = nullptr);
    virtual ~AgeTableView () override { }
    void connectScollBar(QScrollBar *sb);
    void connectZoomSlider(QSlider *zoomSlider);
    void connectLabels(QLabel *minTsLabel, QLabel *maxTsLabel, QLabel *totalSizeLabel);
    void connectGridlinesToggle(QCheckBox *checkBox);

signals:
    void setScrollMax(int newMax);
    void setZoomRange(int newMin, int newMax);
    void setMinLabel(QString newText);
    void setMaxLabel(QString newText);
    void setTotalSizeLabel(QString newText);
    void doubleClickedName(QString name);
    void clickedEmptyTable();

public slots:
    void numBinsChanged(int newNumBins);
    void histogramScroll(int newScrollValue);
    void histogramZoom(int newZoomValue);
    void toggleGridlines(int state);

    // QAbstractItemView interface
public:
    virtual void setModel(QAbstractItemModel *model) override;

    // QAbstractItemView interface
protected slots:
    virtual void rowsInserted(const QModelIndex &parent, int start, int end) override;
};

#endif // AGETABLEVIEW_H
