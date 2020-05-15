#include "agemodel.h"
#include <QDir>
#include <QDebug>

const int NUM_BUCKETS_DEFAULT = 50;

AgeModel::AgeModel(QObject *parent)
    :QAbstractTableModel(parent)
    ,m_numBins(NUM_BUCKETS_DEFAULT)
{
}

void AgeModel::checkExpandMinMaxTimestamps(qint64 new_min_ts, qint64 new_max_ts)
{
    bool changed = false;
    if (!m_minModelTimestamp.isValid() || m_minModelTimestamp.get() > new_min_ts) {
        m_minModelTimestamp.set(new_min_ts);
        changed = true;
    }
    if (!m_maxModelTimestamp.isValid() || m_maxModelTimestamp.get() < new_max_ts) {
        m_maxModelTimestamp.set(new_max_ts);
        changed = true;
    }
    if (changed) {
        this->rebuildHistograms();
        emit minMaxTimestampChanged(m_minModelTimestamp.get(), m_maxModelTimestamp.get());
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
    emit dataChanged(QModelIndex(), QModelIndex());
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
            return QVariant::fromValue(index.row());
        }
    }
    return QVariant();
}

QVariant AgeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case 0:
                return "Name";
            case 1:
                return "Age";
            }
        }
    }
    return QVariant();
}

qint64 AgeModel::largestBinSize(int fromIndex, int toIndex) const
{
    qint64 largestBin = 0;
    for (auto i = m_rows.cbegin(); i != m_rows.cend(); ++i) {
        qint64 j = (*i).histogram.largestBigSize(fromIndex, toIndex);
        if (largestBin < j) {
            largestBin = j;
        }
    }
    return largestBin;
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

void AgeModel::clear()
{
    this->beginRemoveRows(QModelIndex(), 0, m_rows.count()-1);
    m_rows.clear();
    m_minModelTimestamp.clear();
    m_maxModelTimestamp.clear();
    this->endRemoveRows();
}
