#include "agetableview.h"
#include <QScrollBar>
#include <QHeaderView>
#include <QMouseEvent>

const int DEFAULT_VISIBLE_BINS = 50;
const int MIN_VISIBLE_BINS = 5;

qint64 AgeTableView::largestBinSizeInView(AgeModel *model)
{
    // have AgeModel as parameter to avoid downcasting multiple times
    const int firstBin = m_histogramDelegate.firstVisibleBin();
    const int lastBin = firstBin + m_histogramDelegate.numVisibleBins() - 1;
    return model->largestBinSize(firstBin, lastBin);
}

void AgeTableView::updateLabels(AgeModel *model)
{
    const int firstBin = m_histogramDelegate.firstVisibleBin();
    const int lastBin = firstBin + m_histogramDelegate.numVisibleBins() - 1;
    if (model->rowCount() > 0 && model->timestampsAreValid()) {
        qint64 minTs = model->binRange(firstBin).first;
        QDateTime minDate = QDateTime::fromSecsSinceEpoch(minTs);
        emit setMinLabel(minDate.toString());
        qint64 maxTs = model->binRange(lastBin).second;
        QDateTime maxDate = QDateTime::fromSecsSinceEpoch(maxTs);
        emit setMaxLabel(maxDate.toString());
    } else {
        emit setMinLabel(QString());
        emit setMaxLabel(QString());
    }
}

void AgeTableView::mouseDoubleClickEvent(QMouseEvent *event)
{
    int row = this->rowAt(event->y());
    const AgeModel *am = qobject_cast<const AgeModel*>(this->model());
    if (am != nullptr) {
        QVariant value = am->data(am->index(row, AgeModel::COLUMN_AGE));
        QString name = value.value<AgeModelRow>().name;
        emit doubleClickedName(name);
    }
}

AgeTableView::AgeTableView(QWidget *parent)
    :QTableView(parent)
    ,m_histogramDelegate(this)
{
    m_histogramDelegate.setNumVisibleBins(DEFAULT_VISIBLE_BINS);
    this->setSortingEnabled(true);
}

void AgeTableView::connectScollBar(QScrollBar *sb)
{
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr) {
        m_histogramDelegate.setLargestBinInView(this->largestBinSizeInView(am));
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

void AgeTableView::connectLabels(QLabel *minTsLabel, QLabel *maxTsLabel)
{
    connect(this, &AgeTableView::setMinLabel, minTsLabel, &QLabel::setText);
    connect(this, &AgeTableView::setMaxLabel, maxTsLabel, &QLabel::setText);
}

void AgeTableView::numBinsChanged(int newNumBins)
{
    emit setScrollMax(newNumBins - 1);
    emit setZoomRange(MIN_VISIBLE_BINS, newNumBins);
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr) {
        m_histogramDelegate.setLargestBinInView(this->largestBinSizeInView(am));
        this->updateLabels(am);
    }
    this->viewport()->update();
}

void AgeTableView::histogramScroll(int newScrollValue)
{
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr && newScrollValue < am->numBins()) {
        m_histogramDelegate.setFirstVisibleBin(newScrollValue);
        m_histogramDelegate.setLargestBinInView(this->largestBinSizeInView(am));
        this->updateLabels(am);
    }
    this->viewport()->update();
}

void AgeTableView::histogramZoom(int newZoomValue)
{
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr && newZoomValue >= 0 && newZoomValue <= am->numBins()) {
        m_histogramDelegate.setNumVisibleBins(newZoomValue);
        m_histogramDelegate.setLargestBinInView(this->largestBinSizeInView(am));
        this->updateLabels(am);
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
            m_histogramDelegate.setLargestBinInView(this->largestBinSizeInView(am));
            this->updateLabels(am);
        }
    }
}
