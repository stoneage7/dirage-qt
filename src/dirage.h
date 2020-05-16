#ifndef DIRAGE_H
#define DIRAGE_H

#include <QMainWindow>
#include <QFileSystemModel>
#include "agemodel.h"
#include "ageitemdelegate.h"
#include "scanner.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DirAge; }
QT_END_NAMESPACE

class DirAge : public QMainWindow
{
    Q_OBJECT

public:
    DirAge(QWidget *parent = nullptr);
    ~DirAge();

private:
    Ui::DirAge *ui;
    AgeModel m_ageModel;
    SubdirsScanner *m_scanner;
    QScopedPointer<QTimer> m_scannerPing;
    QScopedPointer<QDir> m_path;
    enum ScanButtonState { ScanButtonOpen, ScanButtonCancel };

    void changeScanButtonState(DirAge::ScanButtonState new_state);

public slots:
    void openDirDialog();
    void upDir();
    void runScan(QString path);
    void stopScanner();
    void updateScanningStatus(QString current_path);
    void updateMinMaxTimestamp(qint64 min_ts, qint64 max_ts);
};



#endif // DIRAGE_H
