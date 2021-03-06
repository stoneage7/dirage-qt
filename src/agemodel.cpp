#include "agemodel.h"
#include "platform.h"
#include <QDir>

const int NUM_BUCKETS_DEFAULT = 50;

AgeModel::AgeModel(QObject *parent)
    :QAbstractTableModel(parent)
    ,m_numBins(NUM_BUCKETS_DEFAULT)
    ,m_totalSize(0)
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

void AgeModel::appendRowFromVector(QString name, QString label, AgeVector vector)
{
    if (!vector.datapoints().isEmpty()) {
        this->checkExpandMinMaxTimestamps(vector.minTimestamp(), vector.maxTimestamp());
    }
    AgeModelRow it;
    it.name = name;
    it.label = label;
    it.vector = vector;
    it.histogram = this->makeHistogram(vector);
    this->beginInsertRows(QModelIndex(), m_rows.count(), m_rows.count());
    m_rows.append(it);
    m_totalSize += it.histogram.sumBins();
    this->endInsertRows();
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
    return parent.isValid() ? 0 : COLUMN_COUNT;
}

QVariant AgeModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case COLUMN_NAME:
            return QVariant(m_rows.at(index.row()).label);
        case COLUMN_SIZE:
            return QVariant(m_rows.at(index.row()).histogram.sumBins());
        case COLUMN_AGE:
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
                return tr("Name");
            case COLUMN_SIZE:
                return tr("Size");
            case COLUMN_AGE:
                return tr("Age");
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
            bool less = a.name.compare(b.name) < 0;
            return asc ? less : !less;
        });
        break;
    case COLUMN_SIZE:
        std::stable_sort(m_rows.begin(), m_rows.end(),
                         [asc] (const AgeModelRow &a, const AgeModelRow &b) -> bool {
            bool less = a.histogram.sumBins() < b.histogram.sumBins();
            return asc ? less : !less;
        });
        break;
    case COLUMN_AGE:
        std::stable_sort(m_rows.begin(), m_rows.end(),
                         [asc] (const AgeModelRow &a, const AgeModelRow &b) -> bool {
            bool less = false;
            const bool aEmpty = a.histogram.bins().isEmpty();
            const bool bEmpty = b.histogram.bins().isEmpty();
            if (aEmpty && !bEmpty) {
                less = true;
            } else if (aEmpty && bEmpty) {
                less = a.name.compare(b.name) < 0;
            } else if (!aEmpty && !bEmpty) {
                less = a.histogram.medianTimestamp().get() < b.histogram.medianTimestamp().get();
            }
            return asc ? less : !less;
        });
        break;
    }
    emit dataChanged(this->createIndex(0, 0), this->createIndex(this->rowCount()-1,
                                                                this->columnCount()-1));
}

std::pair<qint64, qint64> AgeModel::binRange(int binIndex) const
{
    return platform::histogramImpl->binRange(this->minTimestamp(), this->maxTimestamp(),
                                             this->numBins(), binIndex);
}

std::pair<qint64, qint64> AgeModel::largestBinAndSum(int fromIndex, int toIndex) const
{
    Q_ASSERT(fromIndex >= 0 && toIndex >= 0);
    qint64 largestBin = 0;
    qint64 sum = 0;
    for (auto i = m_rows.cbegin(); i != m_rows.cend(); ++i) {
        auto &bins = (*i).histogram.bins();
        if (bins.length() > 0) {
            int tmpFrom = qMin(fromIndex, bins.length()-1);
            int tmpTo = qMin(toIndex, bins.length()-1);
            std::pair<qint64, qint64> s = (*i).histogram.largestBinAndSum(tmpFrom, tmpTo);
            if (largestBin < s.first) {
                largestBin = s.first;
            }
            sum += s.second;
        }
    }
    return std::pair<qint64, qint64> (largestBin, sum);
}

void AgeModel::appendSubdir(QString name, AgeVector vector)
{
    this->appendRowFromVector(name, QDir(name).dirName(), vector);
}

void AgeModel::appendTopLevel(QString name, AgeVector vector)
{
    this->appendRowFromVector(name, QStringLiteral("./"), vector);
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
    m_totalSize = 0;
    this->endRemoveRows();
}
