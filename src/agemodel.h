#ifndef AGEMODEL_H
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
        COLUMN_AGE,
        COLUMN_COUNT
    };

private:
    QVector<AgeModelRow> m_rows;
    int m_numBins;
    TimestampOption m_minModelTimestamp;
    TimestampOption m_maxModelTimestamp;

    void checkExpandMinMaxTimestamps(qint64 newMinTs, qint64 newMaxTs);
    void chackUpdateChartHeights(const AgeHistogram &beingAdded);
    AgeHistogram makeHistogram(const AgeVector &vector);
    void rebuildHistograms();

public:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    int numBins() const { return m_numBins; }
    qint64 largestBinSize(int fromIndex, int toIndex) const;

signals:
    void minMaxTimestampChanged(qint64 min_ts, qint64 max_ts);
    void numBinsChanged(int newNumBins);

public slots:
    void insertOrChangeAge(QString name, AgeVector vector);
    void setNumBins(int newNumBins);
    void clear();
};

#endif // AGEMODEL_H
