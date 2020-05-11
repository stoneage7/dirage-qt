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

struct AgeModelGlobalState {
    int numBuckets;
    TimestampOption minModelTimestamp;
    TimestampOption maxModelTimestamp;
    int pan;
    int zoom;
    TimestampOption minZoomTimestamp;
    TimestampOption maxZoomTimestamp;
    qint64 maxBinSize;
    AgeModelGlobalState(): numBuckets(0), pan(50),  zoom(50), maxBinSize(0) { }
};

// returned by AgeModel::data(). AgeItemDelegate uses this to draw the chart with correct size.
struct AgeRenderData {
    AgeModelRow row;
    AgeModelGlobalState global;
};
Q_DECLARE_METATYPE(AgeRenderData);

class AgeModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    AgeModel(QObject *parent);

private:
    QVector<AgeModelRow> m_rows;
    AgeModelGlobalState m_global;

    void applyPanAndZoom();
    void checkExpandMinMaxTimestamps(qint64 new_min_ts, qint64 new_max_ts);
    void checkRefreshRowHeights(qint64 new_max_size);
    void rebuildHistograms();

public:
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;

signals:
    void minMaxTimestampChanged(qint64 min_ts, qint64 max_ts);

public slots:
    void insertOrChangeAge(QString name, AgeVector vector);
    void clear();
    void changePan(int new_pan);
    void changeZoom(int new_zoom);
};

#endif // AGEMODEL_H
