﻿#ifndef AGEMODEL_H
#define AGEMODEL_H

#include <QAbstractTableModel>
#include "agehistogram.h"

struct AgeModelRow {
    QString name;
    QString label;
    AgeVector vector;
    AgeHistogram histogram;
};
Q_DECLARE_METATYPE(AgeModelRow);

class AgeModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    AgeModel(QObject *parent);
    enum Columns {
        COLUMN_NAME,
        COLUMN_SIZE,
        COLUMN_AGE,
        COLUMN_COUNT
    };

private:
    QVector<AgeModelRow> m_rows;
    int m_numBins;
    TimestampOption m_minModelTimestamp;
    TimestampOption m_maxModelTimestamp;
    qint64 m_totalSize;

    void checkExpandMinMaxTimestamps(qint64 newMinTs, qint64 newMaxTs);
    void appendRowFromVector(QString name, QString label, AgeVector vector);
    AgeHistogram makeHistogram(const AgeVector &vector);
    void rebuildHistograms();

public:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    virtual void sort(int column, Qt::SortOrder order);

    int numBins() const { return m_numBins; }
    qint64 minTimestamp() const { return m_minModelTimestamp.get(); }
    qint64 maxTimestamp() const { return m_maxModelTimestamp.get(); }
    bool timestampsAreValid() const { return m_minModelTimestamp.isValid() && m_maxModelTimestamp.isValid(); }
    std::pair<qint64, qint64> binRange(int binIndex) const;
    qint64 totalSize() const { return m_totalSize; }
    std::pair<qint64, qint64> largestBinAndSum(int fromIndex, int toIndex) const;

signals:
    void minMaxTimestampChanged(qint64 min_ts, qint64 max_ts);
    void numBinsChanged(int newNumBins);

public slots:
    void appendSubdir(QString name, AgeVector vector);
    void appendTopLevel(QString name, AgeVector vector);
    void setNumBins(int newNumBins);
    void clear();
};

#endif // AGEMODEL_H
