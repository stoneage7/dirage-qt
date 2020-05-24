#include "agetableview.h"
#include <QScrollBar>
#include <QHeaderView>
#include <QMouseEvent>

const int DEFAULT_VISIBLE_BINS = 50;
const int MIN_VISIBLE_BINS = 5;

QString ByteSizeDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    Q_UNUSED(locale)
    qint64 bytes = value.value<qint64>();
    return ByteSizeDelegate::byteSizetoString(bytes);
}

QString ByteSizeDelegate::byteSizetoString(qint64 bytes)
{
    const QString units[] = { QStringLiteral(" B"), QStringLiteral(" kiB"), QStringLiteral(" MiB"),
                              QStringLiteral(" GiB") };
    size_t i = 0;
    qreal bytesF = static_cast<qreal>(bytes);
    while (i < sizeof(units) / sizeof(units[0]) && bytesF > 1024.0) {
        i++;
        bytesF /= 1024.0;
    }
    return QStringLiteral("%1 %2").arg(bytesF, 0, 'f', 2).arg(units[i]);
}

void AgeTableView::updateLargestAndSum(AgeModel *model)
{
    // have AgeModel as parameter to avoid downcasting multiple times
    const int firstBin = m_histogramDelegate.firstVisibleBin();
    const int lastBin = firstBin + m_histogramDelegate.numVisibleBins() - 1;
    //m_histogramDelegate.setLargestBinInView()

    std::pair<qint64, qint64> p = model->largestBinAndSum(firstBin, lastBin);
    m_histogramDelegate.setLargestBinInView(p.first);
    m_sumValuesInView = p.second;
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

    QString totalSizeText(ByteSizeDelegate::byteSizetoString(model->totalSize()));
    QString sizeInViewText(ByteSizeDelegate::byteSizetoString(m_sumValuesInView));
    QString rowHeightText(ByteSizeDelegate::byteSizetoString(m_histogramDelegate.largestBinInView()));
    emit setTotalSizeLabel(tr("Total Size: %1 (%2 in view), Row Height = %3")
                           .arg(totalSizeText).arg(sizeInViewText).arg(rowHeightText));
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
    ,m_sumValuesInView(0)
{
    m_histogramDelegate.setNumVisibleBins(DEFAULT_VISIBLE_BINS);
    this->setSortingEnabled(true);
}

void AgeTableView::connectScollBar(QScrollBar *sb)
{
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr) {
        this->updateLargestAndSum(am);
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

void AgeTableView::connectLabels(QLabel *minTsLabel, QLabel *maxTsLabel, QLabel *totalSizeLabel)
{
    connect(this, &AgeTableView::setMinLabel, minTsLabel, &QLabel::setText);
    connect(this, &AgeTableView::setMaxLabel, maxTsLabel, &QLabel::setText);
    connect(this, &AgeTableView::setTotalSizeLabel, totalSizeLabel, &QLabel::setText);
}

void AgeTableView::connectGridlinesToggle(QCheckBox *checkBox)
{
    connect(checkBox, &QCheckBox::stateChanged, this, &AgeTableView::toggleGridlines);
}

void AgeTableView::numBinsChanged(int newNumBins)
{
    emit setScrollMax(newNumBins - 1);
    emit setZoomRange(MIN_VISIBLE_BINS, newNumBins);
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr) {
        this->updateLargestAndSum(am);
        this->updateLabels(am);
    }
    this->viewport()->update();
}

void AgeTableView::histogramScroll(int newScrollValue)
{
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr && newScrollValue < am->numBins()) {
        m_histogramDelegate.setFirstVisibleBin(newScrollValue);
        this->updateLargestAndSum(am);
        this->updateLabels(am);
    }
    this->viewport()->update();
}

void AgeTableView::histogramZoom(int newZoomValue)
{
    AgeModel *am = qobject_cast<AgeModel*>(this->model());
    if (am != nullptr && newZoomValue >= 0 && newZoomValue <= am->numBins()) {
        m_histogramDelegate.setNumVisibleBins(newZoomValue);
        this->updateLargestAndSum(am);
        this->updateLabels(am);
    }
    this->viewport()->update();
}

void AgeTableView::toggleGridlines(int state)
{
    m_histogramDelegate.setGridLinesToggle(state);
    this->viewport()->update();
}

void AgeTableView::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
    AgeModel *am = qobject_cast<AgeModel*>(model);
    if (am != nullptr) {
        connect(am, &AgeModel::numBinsChanged, this, &AgeTableView::numBinsChanged);
        this->setItemDelegateForColumn(am->COLUMN_SIZE, &m_byteSizeDelegate);
        this->setItemDelegateForColumn(am->COLUMN_AGE, &m_histogramDelegate);
    }
    this->horizontalHeader()->setSectionResizeMode(AgeModel::COLUMN_AGE, QHeaderView::Stretch);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}


void AgeTableView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(start)
    Q_UNUSED(end)
    if (!parent.isValid()) {
        AgeModel *am = qobject_cast<AgeModel*>(this->model());
        if (am != nullptr) {
            this->updateLargestAndSum(am);
            this->updateLabels(am);
        }
    }
}
