#include "agemodel.h"
#include <QDir>
#include <QDebug>

const int NUM_BUCKETS_DEFAULT = 50;

AgeModel::AgeModel(QObject *parent):
    QAbstractTableModel(parent)
{
    m_global.numBuckets = NUM_BUCKETS_DEFAULT;
    m_global.maxBinSize = 0;
}

void AgeModel::applyPanAndZoom()
{
    if (!m_global.minModelTimestamp.isValid() || !m_global.maxModelTimestamp.isValid()) {
        return;
    }
    qint64 transpose = 0;
    qint64 zoom_in = 0;
    if (m_global.pan != 50) {
        transpose = (m_global.maxModelTimestamp.get() - m_global.minModelTimestamp.get())
                * (m_global.pan - 50) / 100;
    }
    if (m_global.zoom != 50) {
        zoom_in = (m_global.maxModelTimestamp.get() - m_global.minModelTimestamp.get())
                * (m_global.zoom - 50) / 110;
    }
    m_global.minZoomTimestamp.set(m_global.minModelTimestamp.get() + transpose + zoom_in);
    m_global.maxZoomTimestamp.set(m_global.maxModelTimestamp.get() + transpose - zoom_in);
            /*
             * if pan == 0 then viewleft = modelleft + range * (pan-50) / 100
             * if pan == 50 then viewleft = modelleft
             * if pan == 100 then viewleft = modelleft + range / 2
             * */
}

void AgeModel::checkExpandMinMaxTimestamps(qint64 new_min_ts, qint64 new_max_ts)
{
    bool changed = false;
    if (!m_global.minModelTimestamp.isValid() || m_global.minModelTimestamp.get() > new_min_ts) {
        m_global.minModelTimestamp.set(new_min_ts);
        changed = true;
    }
    if (!m_global.maxModelTimestamp.isValid() || m_global.maxModelTimestamp.get() < new_max_ts) {
        m_global.maxModelTimestamp.set(new_max_ts);
        changed = true;
    }
    if (changed) {
        this->applyPanAndZoom();
        this->rebuildHistograms();
        emit minMaxTimestampChanged(m_global.minZoomTimestamp.get(), m_global.maxZoomTimestamp.get());
    }
}

void AgeModel::checkRefreshRowHeights(qint64 new_max_size)
{
    if (m_global.maxBinSize < new_max_size) {
        m_global.maxBinSize = new_max_size;
        emit dataChanged(QModelIndex(), QModelIndex());
    }
}

void AgeModel::rebuildHistograms()
{
    qint64 new_max_bin_size = 0;
    for (auto i = m_rows.begin(); i != m_rows.end(); ++i) {
        if ((*i).vector.isEmpty()) {
            (*i).histogram = AgeHistogram();
        } else {
            (*i).histogram = AgeHistogram((*i).vector, m_global.numBuckets,
                                          m_global.minZoomTimestamp.get(),
                                          m_global.maxZoomTimestamp.get());
            if ((*i).histogram.maxBinSize() > new_max_bin_size) {
                new_max_bin_size = (*i).histogram.maxBinSize();
            }
        }
    }
    m_global.maxBinSize = new_max_bin_size;
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
        case 0: return QVariant(m_rows.at(index.row()).label);
        case 1:
            AgeRenderData rv;
            rv.row = m_rows.at(index.row());
            rv.global = m_global;
            return QVariant::fromValue(rv);
        }
    }
    return QVariant();
}

QVariant AgeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch (section) {
            case 0: return "Name";
            case 1: return "Age";
            }
        }
    }
    return QVariant();
}
// TODO implement CHANGE
void AgeModel::insertOrChangeAge(QString name, AgeVector vector)
{
    if (!vector.isEmpty()) {
        this->checkExpandMinMaxTimestamps(vector.minTimestamp(), vector.maxTimestamp());
    }
    AgeModelRow it;
    it.name = name;
    it.label = QDir(name).dirName();
    it.vector = vector;
    if (vector.isEmpty()) {
        it.histogram = AgeHistogram();
    } else {
        it.histogram = AgeHistogram(vector, m_global.numBuckets,
                                    m_global.minZoomTimestamp.get(), m_global.maxZoomTimestamp.get());
    }
    checkRefreshRowHeights(it.histogram.maxBinSize());
    this->beginInsertRows(QModelIndex(), m_rows.count(), m_rows.count());
    m_rows.append(it);
    this->endInsertRows();
}

void AgeModel::clear()
{
    this->beginRemoveRows(QModelIndex(), 0, m_rows.count()-1);
    m_rows.clear();
    m_global.minZoomTimestamp.clear();
    m_global.maxZoomTimestamp.clear();
    m_global.minModelTimestamp.clear();
    m_global.maxModelTimestamp.clear();
    m_global.maxBinSize = 0;
    this->endRemoveRows();
}

void AgeModel::changePan(int new_pan)
{
    if (m_global.pan != new_pan) {
        m_global.pan = new_pan;
        if (m_global.minZoomTimestamp.isValid() && m_global.maxZoomTimestamp.isValid()) {
            this->applyPanAndZoom();
            this->rebuildHistograms();
            emit minMaxTimestampChanged(m_global.minZoomTimestamp.get(),
                                        m_global.maxZoomTimestamp.get());
        }
    }
}

void AgeModel::changeZoom(int new_zoom)
{
    if (m_global.zoom != new_zoom) {
        m_global.zoom = new_zoom;
        if (m_global.minZoomTimestamp.isValid() && m_global.maxZoomTimestamp.isValid()) {
            this->applyPanAndZoom();
            this->rebuildHistograms();
            emit minMaxTimestampChanged(m_global.minZoomTimestamp.get(),
                                        m_global.maxZoomTimestamp.get());
        }
    }

}
