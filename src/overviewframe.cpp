// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QClipboard>
#include <QPushButton>
#include <QAbstractItemModel>
#include <QMouseEvent>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QProgressDialog>
#include <QDesktopServices>


#include "overviewframe.h"
#include "walletmodel.h"
#include "rpcapi.h"

#include "ui_overviewframe.h"

namespace WalletGUI {

namespace {

const char MAIN_TX_HASH_URL[] = "https://explorer.bytecoin.org/tx?hash=%1";
const char MAIN_BLOCK_HASH_URL[] = "https://explorer.bytecoin.org/block?hash=%1";
const char MAIN_BLOCK_HEIGHT_URL[] = "https://explorer.bytecoin.org/block?height=%1";
const char STAGE_TX_HASH_URL[] = "https://stage.explorer.bytecoin.org/tx?hash=%1";
const char STAGE_BLOCK_HASH_URL[] = "https://stage.explorer.bytecoin.org/block?hash=%1";
const char STAGE_BLOCK_HEIGHT_URL[] = "https://stage.explorer.bytecoin.org/block?height=%1";


//const char OVERVIEW_STYLE_SHEET_TEMPLATE[] =
//  "* {"
//    "font-family: %fontFamily%;"
//  "}"

//  "WalletGui--OverviewFrame {"
//    "background-color: #ffffff;"
//    "border: none;"
//  "}"

//  "WalletGui--OverviewFrame #m_allTransactionsButton {"
//    "margin-top: 2px;"
//  "}"

//  "WalletGui--OverviewFrame #m_newsFrame {"
//    "min-height: 130px;"
//    "max-height: 130px;"
//    "border: none;"
//    "border-top: 1px solid %borderColor%;"
//    "background-color: %backgroundColorGray%;"
//  "}";

}

//const QMap<int, int> indices =
//        {{ WalletModel::COLUMN_AMOUNT, 0 },
//         { WalletModel::COLUMN_FEE, 1 },
//         { WalletModel::COLUMN_HASH, 2 },
//         { WalletModel::COLUMN_BLOCK_HEIGHT, 3 },
//         { WalletModel::COLUMN_BLOCK_HASH, 4 },
//         { WalletModel::COLUMN_TIMESTAMP, 5 },
//         { WalletModel::COLUMN_UNLOCK_TIME, 6 },
//         { WalletModel::COLUMN_PROOF, 7}};

const QList<int> columns_order =
    { WalletModel::COLUMN_AMOUNT,
      WalletModel::COLUMN_FEE,
      WalletModel::COLUMN_HASH,
      WalletModel::COLUMN_BLOCK_HASH,
      WalletModel::COLUMN_TIMESTAMP,
      WalletModel::COLUMN_BLOCK_HEIGHT,
      WalletModel::COLUMN_UNLOCK_TIME,
      WalletModel::COLUMN_PROOF };

OverviewFrame::OverviewFrame(QWidget* parent)
    : QFrame(parent)
    , m_ui(new Ui::OverviewFrame)
    , m_mainWindow(nullptr)
    , m_csvExporter(nullptr)
{
    m_ui->setupUi(this);

    m_ui->m_recentTransactionsView->viewport()->setMouseTracking(true);
    m_ui->m_recentTransactionsView->viewport()->installEventFilter(this);

    connect(m_ui->m_balanceOverviewFrame, &BalanceOverviewFrame::copiedToClipboardSignal, this, &OverviewFrame::copiedToClipboardSignal);
}

OverviewFrame::~OverviewFrame()
{}

void OverviewFrame::setTransactionsModel(WalletModel* model)
{
    m_transactionsModel = model;
    m_ui->m_recentTransactionsView->setModel(model);

    const int columns = m_transactionsModel->columnCount();
    // hide all columns
    for (int i = 0; i < columns; ++i)
        m_ui->m_recentTransactionsView->setColumnHidden(i, true);

    QHeaderView& header = *m_ui->m_recentTransactionsView->horizontalHeader();
    header.setResizeContentsPrecision(-1);

    for (int i = 0; i < columns_order.size(); ++i)
    {
        const int column = columns_order[i];
        header.moveSection(header.visualIndex(column), i);
        header.setSectionResizeMode(column, (column == WalletModel::COLUMN_HASH || column == WalletModel::COLUMN_BLOCK_HASH) ? QHeaderView::Stretch : QHeaderView::ResizeToContents);

        // then unhide needed ones
        m_ui->m_recentTransactionsView->setColumnHidden(column, false);
    }

    delete m_csvExporter;
    m_csvExporter = new CSVTransactionsExporter(model, this);
}

void OverviewFrame::setWalletModel(WalletModel* walletModel)
{
    m_ui->m_balanceOverviewFrame->setWalletModel(walletModel);
    m_ui->m_miningOverviewFrame->setWalletModel(walletModel);
}

void OverviewFrame::setMiningManager(MiningManager* miningManager)
{
    m_ui->m_miningOverviewFrame->setMiningManager(miningManager);
}

void OverviewFrame::setMinerModel(QAbstractItemModel* model)
{
    m_ui->m_miningOverviewFrame->setMinerModel(model);
}

void OverviewFrame::rowsInserted(const QModelIndex& /*parent*/, int /*first*/, int /*last*/)
{
//    m_ui->m_recentTransactionsView->resizeColumnsToContents();
}

void OverviewFrame::setMainWindow(QWidget* mainWindow)
{
    m_mainWindow = mainWindow;
}

void OverviewFrame::exportToCSV()
{
    m_csvExporter->start();
}

bool OverviewFrame::eventFilter(QObject* object, QEvent* event)
{
    const bool needToProcess =
            object == m_ui->m_recentTransactionsView->viewport()
            && (event->type() == QEvent::MouseButtonRelease
                || event->type() == QEvent::MouseMove
                || event->type() == QEvent::Leave);
    if (!needToProcess)
        return false;

    QWidget* widget = qobject_cast<QWidget*>(object);
    if(!widget)
        return false;
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(widget->parent());
    if(!view)
        return false;

    if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* e = (QMouseEvent*)event;
        QModelIndex modelIndex = view->indexAt(e->pos());
        if (!modelIndex.isValid())
            return false;
        const QString net = m_transactionsModel->data(modelIndex, WalletModel::ROLE_NET).toString();
        const bool isTestnet = (net == RpcApi::TEST_NET_NAME);
        const bool explorableColumns =
                !isTestnet &&
                ((modelIndex.column() == WalletModel::COLUMN_HASH && !m_transactionsModel->data(modelIndex, WalletModel::ROLE_HASH).toString().isEmpty()) ||
                (modelIndex.column() == WalletModel::COLUMN_BLOCK_HASH && !m_transactionsModel->data(modelIndex, WalletModel::ROLE_BLOCK_HASH).toString().isEmpty()) ||
                (modelIndex.column() == WalletModel::COLUMN_BLOCK_HEIGHT && !m_transactionsModel->data(modelIndex, WalletModel::ROLE_BLOCK_HEIGHT).toString().isEmpty()));
        const bool proof = m_transactionsModel->data(modelIndex, WalletModel::ROLE_PROOF).toBool();
        const bool proofColumn = (modelIndex.column() == WalletModel::COLUMN_PROOF && proof);
        const bool valid = explorableColumns || proofColumn;
        if(!valid)
            return false;

        if (explorableColumns)
        {
            const bool mainnet = (net == RpcApi::MAIN_NET_NAME);
            switch(modelIndex.column())
            {
            case WalletModel::COLUMN_HASH:
                QDesktopServices::openUrl(QUrl::fromUserInput(QString{mainnet ? MAIN_TX_HASH_URL : STAGE_TX_HASH_URL}.arg(m_transactionsModel->data(modelIndex, WalletModel::ROLE_HASH).toString())));
                break;
            case WalletModel::COLUMN_BLOCK_HASH:
                QDesktopServices::openUrl(QUrl::fromUserInput(QString{mainnet ? MAIN_BLOCK_HASH_URL : STAGE_BLOCK_HASH_URL}.arg(m_transactionsModel->data(modelIndex, WalletModel::ROLE_BLOCK_HASH).toString())));
                break;
            case WalletModel::COLUMN_BLOCK_HEIGHT:
                QDesktopServices::openUrl(QUrl::fromUserInput(QString{mainnet ? MAIN_BLOCK_HEIGHT_URL : STAGE_BLOCK_HEIGHT_URL}.arg(m_transactionsModel->data(modelIndex, WalletModel::ROLE_BLOCK_HEIGHT).toString())));
                break;
            }
        }
        if (proofColumn)
        {
            const QString txHash = m_transactionsModel->data(modelIndex, WalletModel::ROLE_HASH).toString();
            const QStringList addresses = m_transactionsModel->data(modelIndex, WalletModel::ROLE_RECIPIENTS).toStringList();
            emit createProofSignal(txHash, addresses, !proof);
        }
    }
    else if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent* e = (QMouseEvent*)event;
        QModelIndex modelIndex = view->indexAt(e->pos());

        const QString net = m_transactionsModel->data(modelIndex, WalletModel::ROLE_NET).toString();
        const bool isTestnet = (net == RpcApi::TEST_NET_NAME);
        const bool explorableColumns =
                !isTestnet &&
                ((modelIndex.column() == WalletModel::COLUMN_HASH && !m_transactionsModel->data(modelIndex, WalletModel::ROLE_HASH).toString().isEmpty()) ||
                (modelIndex.column() == WalletModel::COLUMN_BLOCK_HASH && !m_transactionsModel->data(modelIndex, WalletModel::ROLE_BLOCK_HASH).toString().isEmpty()) ||
                (modelIndex.column() == WalletModel::COLUMN_BLOCK_HEIGHT && !m_transactionsModel->data(modelIndex, WalletModel::ROLE_BLOCK_HEIGHT).toString().isEmpty()));
        const bool proof = m_transactionsModel->data(modelIndex, WalletModel::ROLE_PROOF).toBool();
        const bool proofColumn = (modelIndex.column() == WalletModel::COLUMN_PROOF && proof);

        const bool showHandCursor = explorableColumns || proofColumn;

        setCursor(showHandCursor ? Qt::PointingHandCursor : Qt::ArrowCursor);
    }
    else if (event->type() == QEvent::Leave)
        setCursor(Qt::ArrowCursor);

    return false;
}

CSVTransactionsExporter::CSVTransactionsExporter(WalletModel* transactionsModel, QWidget* parent)
    : parent_(parent)
    , file_(this)
    , csv_(&file_)
    , exportProgressDlg_(nullptr)
    , transactionsModel_(transactionsModel)
{
}

void CSVTransactionsExporter::start()
{
    const QString fileName = QFileDialog::getSaveFileName(
                parent_,
                tr("Export to CSV file"),
                QDir::homePath(),
                tr("CSV files (*.csv);;All files (*)"));
    if (fileName.isEmpty())
        return;

    file_.setFileName(fileName);
    if (!file_.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(parent_, tr("Error"), tr("Failed to open the specified file (%1)").arg(fileName));
        return;
    }

    exportProgressDlg_ = new QProgressDialog(parent_);
    exportProgressDlg_->setLabelText(tr("Exporting..."));
    exportProgressDlg_->setWindowModality(Qt::WindowModal);
    exportProgressDlg_->setRange(0, 0);
    exportProgressDlg_->setValue(0);
    exportProgressDlg_->setMinimumDuration(500);
//    exportProgressDlg_->show();

    writeHeader();

    if (transactionsModel_->canFetchMore())
    {
        connect(transactionsModel_, &WalletModel::fetchedSignal, this, &CSVTransactionsExporter::fetched);
        connect(transactionsModel_, &WalletModel::nothingToFetchSignal, this, &CSVTransactionsExporter::finish);
        connect(transactionsModel_, &WalletModel::modelAboutToBeReset, exportProgressDlg_, &QProgressDialog::cancel);
        connect(exportProgressDlg_, &QProgressDialog::canceled, this, &CSVTransactionsExporter::cancel);
        transactionsModel_->fetchMore();
    }
    else
    {
        writeData();
        close();
        QMessageBox::information(parent_, tr("Info"), tr("Transaction history successfully exported."));
    }
}

void CSVTransactionsExporter::writeHeader()
{
    QStringList columns;
    for(int i = 0; i < columns_order.size(); ++i)
    {
        const int column = columns_order[i];
        if (column != WalletModel::COLUMN_PROOF) // skip proof column
            columns << transactionsModel_->headerData(columns_order[i], Qt::Horizontal).toString();
    }
    csv_ << columns.join(';') + '\n';
}

void CSVTransactionsExporter::writeData()
{
    const int rowCount = transactionsModel_->rowCount();
    for(int row = 0; row < rowCount; ++row)
    {
        QStringList columns;
        for(int i = 0; i < columns_order.size(); ++i)
        {
            const int column = columns_order[i];
            if (column != WalletModel::COLUMN_PROOF) // skip proof column
                columns << transactionsModel_->data(transactionsModel_->index(row, column)).toString();
        }
        csv_ << columns.join(';') + '\n';
    }
}

void CSVTransactionsExporter::close()
{
    exportProgressDlg_->setMaximum(1);
    exportProgressDlg_->setValue(exportProgressDlg_->maximum());
    exportProgressDlg_->deleteLater();
    file_.close();
}

void CSVTransactionsExporter::fetched()
{
    if (transactionsModel_->canFetchMore())
        transactionsModel_->fetchMore();
}

void CSVTransactionsExporter::reset()
{
    disconnect(transactionsModel_, &WalletModel::fetchedSignal, this, &CSVTransactionsExporter::fetched);
    disconnect(transactionsModel_, &WalletModel::nothingToFetchSignal, this, &CSVTransactionsExporter::finish);
    disconnect(transactionsModel_, &WalletModel::modelAboutToBeReset, exportProgressDlg_, &QProgressDialog::cancel);
    disconnect(exportProgressDlg_, &QProgressDialog::canceled, this, &CSVTransactionsExporter::cancel);

    writeData();
    close();
}

void CSVTransactionsExporter::finish()
{
    reset();
    QMessageBox::information(parent_, tr("Info"), tr("Transaction history successfully exported."));
}

void CSVTransactionsExporter::cancel()
{
    reset();
    QMessageBox::information(parent_, tr("Info"), tr("Operation aborted."));
}

}
