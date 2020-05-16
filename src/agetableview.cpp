#include "agetableview.h"
#include <QScrollBar>

const int DEFAULT_VISIBLE_BINS = 50;
const int MIN_VISIBLE_BINS = 5;

AgeTableView::AgeTableView(QWidget *parent)
    :QTableView(parent)
    ,m_histogramDelegate(this)
{
    m_histogramDelegate.setNumVisibleBins(DEFAULT_VISIBLE_BINS, nullptr);
}

void AgeTableView::connectScollBar(QScrollBar *sb)
{
    sb->setMinimum(0);
    connect(this, &AgeTableView::setScrollMax, sb, &QScrollBar::setMaximum);
    connect(sb, &QScrollBar::valueChanged, this, &AgeTableView::histogramScroll);
}

void AgeTableView::connectZoomSlider(QAbstractSlider *zoomSlider)
{
    zoomSlider->setRange(MIN_VISIBLE_BINS, DEFAULT_VISIBLE_BINS);
    connect(zoomSlider, &QAbstractSlider::valueChanged, this, &AgeTableView::histogramZoom);
    connect(this, &AgeTableView::setZoomRange, zoomSlider, &QAbstractSlider::setRange);
}

void AgeTableView::numBinsChanged(int newNumBins)
{
    emit setScrollMax(newNumBins - 1);
    emit setZoomRange(MIN_VISIBLE_BINS, newNumBins);
}

void AgeTableView::histogramScroll(int newScrollValue)
{
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr && newScrollValue < am->numBins()) {
        m_histogramDelegate.setFirstVisibleBin(newScrollValue, am);
    }
}

void AgeTableView::histogramZoom(int newZoomValue)
{
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr && newZoomValue < am->numBins()) {
        m_histogramDelegate.setNumVisibleBins(newZoomValue, am);
    }
}

void AgeTableView::setModel(QAbstractItemModel *model)
{
    AgeModel *am = qobject_cast<AgeModel*>(model);
    if (am != nullptr) {
        connect(am, &AgeModel::numBinsChanged, this, &AgeTableView::numBinsChanged);
    }
    QTableView::setModel(model);
}


void AgeTableView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(start)
    Q_UNUSED(end)
    if (!parent.isValid()) {
        AgeModel *am = qobject_cast<AgeModel*>(this->model());
        if (am != nullptr) {
            m_histogramDelegate.updateLargestBinInView(am);
        }
    }
}
