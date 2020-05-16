#include "agetableview.h"
#include <QScrollBar>
#include <QHeaderView>

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
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr) {
        m_histogramDelegate.updateLargestBinInView(am);
        sb->setRange(0, am->numBins());
        connect(this, &AgeTableView::setScrollMax, sb, &QScrollBar::setMaximum);
        connect(sb, &QScrollBar::valueChanged, this, &AgeTableView::histogramScroll);
    }
}

void AgeTableView::connectZoomSlider(QSlider *zoomSlider)
{
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr) {
        zoomSlider->setRange(MIN_VISIBLE_BINS, DEFAULT_VISIBLE_BINS);
        connect(zoomSlider, &QSlider::valueChanged, this, &AgeTableView::histogramZoom);
        connect(this, &AgeTableView::setZoomRange, zoomSlider, &QAbstractSlider::setRange);
    }
}

void AgeTableView::numBinsChanged(int newNumBins)
{
    emit setScrollMax(newNumBins - 1);
    emit setZoomRange(MIN_VISIBLE_BINS, newNumBins);
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr) {
        m_histogramDelegate.updateLargestBinInView(am);
    }
    this->viewport()->update();
}

void AgeTableView::histogramScroll(int newScrollValue)
{
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr && newScrollValue < am->numBins()) {
        m_histogramDelegate.setFirstVisibleBin(newScrollValue, am);
    }
    this->viewport()->update();
}

void AgeTableView::histogramZoom(int newZoomValue)
{
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr && newZoomValue < am->numBins()) {
        m_histogramDelegate.setNumVisibleBins(newZoomValue, am);
    }
    this->viewport()->update();
}

void AgeTableView::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
    AgeModel *am = qobject_cast<AgeModel*>(model);
    if (am != nullptr) {
        connect(am, &AgeModel::numBinsChanged, this, &AgeTableView::numBinsChanged);
        this->setItemDelegateForColumn(am->COLUMN_AGE, &m_histogramDelegate);
    }
    this->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
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
