#include "dirage.h"
#include "ui_dirage.h"
#include <QTimer>
#include <QFileDialog>

DirAge::DirAge(QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::DirAge)
    ,m_ageModel(this)
    ,m_scanner(nullptr)
    ,m_path(nullptr)
{
    ui->setupUi(this);
    ui->statsTable->setModel(&m_ageModel);
    ui->statsTable->connectScollBar(ui->ageScrollBar);
    ui->statsTable->connectZoomSlider(ui->zoomSlider);
    ui->statsTable->connectLabels(ui->minTimestampLabel, ui->maxTimestampLabel, ui->totalSizeLabel);
    ui->statsTable->connectGridlinesToggle(ui->gridLinesToggle);
    connect(ui->statsTable, &AgeTableView::doubleClickedName, this, &DirAge::runScan);
    connect(ui->statsTable, &AgeTableView::clickedEmptyTable, this, &DirAge::openDirDialog);
    ui->statsTable->sortByColumn(AgeModel::COLUMN_NAME, Qt::AscendingOrder);
    connect(ui->numBinsSlider, &QAbstractSlider::valueChanged, &m_ageModel, &AgeModel::setNumBins);
    connect(ui->upDirButton, &QPushButton::clicked, this, &DirAge::upDir);
    ui->currentDirLabel->clear();
    ui->minTimestampLabel->setText(QStringLiteral());
    ui->maxTimestampLabel->setText(QStringLiteral());
    this->changeScanButtonState(DirAge::ScanButtonOpen);
    this->updateScanningStatus(QString());
}

DirAge::~DirAge()
{
    delete ui;
}

void DirAge::changeScanButtonState(DirAge::ScanButtonState new_state)
{
    switch (new_state) {
    case DirAge::ScanButtonOpen:
        ui->scanButton->disconnect(this);
        connect(ui->scanButton, &QPushButton::clicked, this, &DirAge::openDirDialog);
        ui->scanButton->setText(tr("Scan"));
        return;
    case DirAge::ScanButtonCancel:
        ui->scanButton->disconnect(this);
        ui->scanButton->setText(tr("Cancel"));
        connect(ui->scanButton, &QPushButton::clicked, this, &DirAge::stopScanner);
        return;
    }
}

void DirAge::openDirDialog()
{
    QString d =  QFileDialog::getExistingDirectory(this,
                                                   tr("Select a directory to scan."));
    if (!d.isEmpty()) {
        this->runScan(d);
    }
}

void DirAge::upDir()
{
    if (m_scanner != nullptr) {
        stopScanner();
    }
    if (!m_path.isNull()) {
        m_path->cdUp();
        this->runScan(m_path->canonicalPath());
    }
}

void DirAge::runScan(QString path)
{
    if (m_scanner != nullptr) {
        this->stopScanner();
    }
    m_path.reset(new QDir(path));
    ui->currentDirLabel->setText(path);
    this->setWindowTitle(tr("DirAge - %1 - Scanning").arg(path));
    m_ageModel.clear();
    qInfo() << "scanning" << path;
    m_scanner = new SubdirsScanner(path);
    connect(m_scanner, &SubdirsScanner::haveSubDir, &m_ageModel, &AgeModel::appendSubdir);
    connect(m_scanner, &SubdirsScanner::haveRootDir, &m_ageModel, &AgeModel::appendTopLevel);
    connect(m_scanner, &QThread::finished, this, &DirAge::stopScanner);
    m_scanner->start();
    m_scannerPing.reset(new QTimer());
    connect(m_scannerPing.data(), &QTimer::timeout, m_scanner, &SubdirsScanner::ping);
    connect(m_scanner, &SubdirsScanner::pong, this, &DirAge::updateScanningStatus);
    m_scannerPing->start(1000);
    this->changeScanButtonState(DirAge::ScanButtonCancel);
    this->updateScanningStatus(QString());
}

void DirAge::stopScanner()
{
    m_scanner->requestInterruption();
    m_scanner->disconnect(this);
    m_scanner = nullptr;
    m_scannerPing->stop();
    m_scannerPing->deleteLater();
    m_scannerPing.reset(nullptr);
    this->changeScanButtonState(DirAge::ScanButtonOpen);
    this->updateScanningStatus(QString());
    if (!m_path.isNull() && !m_path->isEmpty()) {
        this->setWindowTitle(tr("DirAge - %1").arg(m_path->path()));
    } else {
        this->setWindowTitle(tr("DirAge"));
    }
    if (ui->statsTable->horizontalHeader()->isSortIndicatorShown()) {
        int previousSort = ui->statsTable->horizontalHeader()->sortIndicatorSection();
        Qt::SortOrder previousOrder = ui->statsTable->horizontalHeader()->sortIndicatorOrder();
        ui->statsTable->sortByColumn(-1, Qt::AscendingOrder);
        ui->statsTable->sortByColumn(previousSort, previousOrder);
    }
}

void DirAge::updateScanningStatus(QString current_path)
{
    if (m_scanner != nullptr) {
        if (current_path.isEmpty()) {
            ui->statusbar->showMessage(tr("Scanning %1").arg(m_scanner->path()));
        } else {
            ui->statusbar->showMessage(tr("Scanning %2").arg(current_path));
        }
    } else {
        ui->statusbar->showMessage(tr("Ready."));
    }

}
