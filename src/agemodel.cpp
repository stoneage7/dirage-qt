#include "agemodel.h"
#include "platform.h"
#include <QDir>

const int NUM_BUCKETS_DEFAULT = 50;

AgeModel::AgeModel(QObject *parent)
    :QAbstractTableModel(parent)
    ,m_numBins(NUM_BUCKETS_DEFAULT)
{
}

void AgeModel::checkExpandMinMaxTimestamps(qint64 newMinTs, qint64 newMaxTs)
{
    bool changed = false;
    if (!m_minModelTimestamp.isValid() || m_minModelTimestamp.get() > newMinTs) {
        m_minModelTimestamp.set(newMinTs);
        changed = true;
    }
    if (!m_maxModelTimestamp.isValid() || m_maxModelTimestamp.get() < newMaxTs) {
        m_maxModelTimestamp.set(newMaxTs);
        changed = true;
    }
    if (changed) {
        this->rebuildHistograms();
        emit minMaxTimestampChanged(m_minModelTimestamp.get(), m_maxModelTimestamp.get());
        emit dataChanged(this->createIndex(0, COLUMN_AGE),
                         this->createIndex(this->rowCount()-1, COLUMN_AGE));
    }
}

AgeHistogram AgeModel::makeHistogram(const AgeVector &vector)
{
    if (vector.datapoints().isEmpty()) {
        return AgeHistogram();
    } else {
        AgeHistogram r(vector, m_numBins,  m_minModelTimestamp.get(), m_maxModelTimestamp.get());
        return r;
    }
}

void AgeModel::rebuildHistograms()
{
    for (auto i = m_rows.begin(); i != m_rows.end(); ++i) {
        (*i).histogram = this->makeHistogram((*i).vector);
    }
}

int AgeModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_rows.count();
}

int AgeModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant AgeModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return QVariant(m_rows.at(index.row()).label);
        case 1:
            return QVariant::fromValue(m_rows.at(index.row()));
        }
    }
    return QVariant();
}

QVariant AgeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case COLUMN_NAME:
                return "Name";
            case COLUMN_AGE:
                return "Age";
            }
        }
    }
    return QVariant();
}

void AgeModel::sort(int column, Qt::SortOrder order)
{
    bool asc = (order == Qt::AscendingOrder);
    switch (column) {
    case COLUMN_NAME:
        std::stable_sort(m_rows.begin(), m_rows.end(),
                         [asc] (const AgeModelRow &a, const AgeModelRow &b) -> bool {
            bool less = (a.name < b.name);
            return asc ? less : !less;
        });
        break;
    case COLUMN_AGE:
        std::stable_sort(m_rows.begin(), m_rows.end(),
                         [asc] (const AgeModelRow &a, const AgeModelRow &b) -> bool {
            bool less = false;
            if (a.histogram.bins().isEmpty() && !b.histogram.bins().isEmpty()) {
                return true;
            } else if (b.histogram.bins().isEmpty()) {
                return false;
            } else {
                less = a.name < b.name;
            }
            less = a.histogram.medianTimestamp().get() < b.histogram.medianTimestamp().get();
            return asc ? less : !less;
        });
        break;
    }
    emit dataChanged(this->createIndex(0, 0), this->createIndex(this->rowCount()-1,
                                                                this->columnCount()-1));
}

qint64 AgeModel::largestBinSize(int fromIndex, int toIndex) const
{
    Q_ASSERT(fromIndex >= 0 && toIndex >= 0);
    qint64 largestBin = 0;
    for (auto i = m_rows.cbegin(); i != m_rows.cend(); ++i) {
        auto &bins = (*i).histogram.bins();
        if (bins.length() > 0) {
            fromIndex = qMin(fromIndex, bins.length()-1);
            toIndex = qMin(toIndex, bins.length()-1);
            qint64 j = (*i).histogram.largestBinSize(fromIndex, toIndex);
            if (largestBin < j) {
                largestBin = j;
            }
        }
    }
    return largestBin;
}

std::pair<qint64, qint64> AgeModel::binRange(int binIndex) const
{
    return platform::histogramImpl->binRange(this->minTimestamp(), this->maxTimestamp(),
                                             this->numBins(), binIndex);
}

// TODO implement CHANGE
void AgeModel::insertOrChangeAge(QString name, AgeVector vector)
{
    if (!vector.datapoints().isEmpty()) {
        this->checkExpandMinMaxTimestamps(vector.minTimestamp(), vector.maxTimestamp());
    }
    AgeModelRow it;
    it.name = name;
    it.label = QDir(name).dirName();
    it.vector = vector;
    it.histogram = this->makeHistogram(vector);
    this->beginInsertRows(QModelIndex(), m_rows.count(), m_rows.count());
    m_rows.append(it);
    this->endInsertRows();
}

void AgeModel::setNumBins(int newNumBins)
{
    newNumBins = qMax(newNumBins, 1);
    m_numBins = newNumBins;
    this->rebuildHistograms();
    emit numBinsChanged(newNumBins);
    emit dataChanged(this->createIndex(0, COLUMN_AGE),
                     this->createIndex(this->rowCount()-1, COLUMN_AGE));
}

void AgeModel::clear()
{
    this->beginRemoveRows(QModelIndex(), 0, m_rows.count()-1);
    m_rows.clear();
    m_minModelTimestamp.clear();
    m_maxModelTimestamp.clear();
    this->endRemoveRows();
}
