#ifndef AGETABLEVIEW_H
#define AGETABLEVIEW_H

#include <QtCore>
#include <QTableView>
#include "ageitemdelegate.h"


class AgeTableView : public QTableView
{
    Q_OBJECT

private:
    AgeItemDelegate m_HistogramDelegate;

public:
    AgeTableView(QWidget *parent = nullptr);
    virtual ~AgeTableView() { }

public slots:
    void setPan(int newPan);
};

#endif // AGETABLEVIEW_H
