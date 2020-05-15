#include "agetableview.h"

const int DEFAULT_VISIBLE_BINS = 50;

AgeTableView::AgeTableView(QWidget *parent)
    :QTableView(parent)
    ,m_HistogramDelegate(this)
{
    m_HistogramDelegate.setNumVisibleBins(DEFAULT_VISIBLE_BINS);
}


