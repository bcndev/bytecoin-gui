// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QFrame>
#include <QFile>
#include <QTextStream>

class QAbstractItemModel;
class QProgressDialog;
class QFile;
class QTextStream;

namespace Ui {
class OverviewFrame;
}

namespace WalletGUI {

class WalletModel;
class MiningManager;
class CopiedToolTip;
class CSVTransactionsExporter;

class OverviewFrame : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(OverviewFrame)

public:
    explicit OverviewFrame(QWidget* parent);
    ~OverviewFrame();

    void setMainWindow(QWidget* mainWindow);
    void setTransactionsModel(WalletModel* transactionsModel);
    void setWalletModel(WalletModel* walletModel);
    void setMiningManager(MiningManager* miningManager);
    void setMinerModel(QAbstractItemModel* model);

public slots:
    void exportToCSV();

signals:
    void copiedToClipboardSignal();
    void createProofSignal(const QString& txHash, const QStringList& addresses, bool needToFind);

private:
    QScopedPointer<Ui::OverviewFrame> m_ui;
    QWidget* m_mainWindow;
    WalletModel* m_transactionsModel;
    CSVTransactionsExporter* m_csvExporter;

    void rowsInserted(const QModelIndex& parent, int first, int last);
    bool eventFilter(QObject* object, QEvent* event) override;
};

class CSVTransactionsExporter : public QObject
{
    Q_OBJECT

public:
    CSVTransactionsExporter(WalletModel* transactionsModel, QWidget* parent);

    void start();
    void cancel();

private:
    QWidget* parent_;
    QFile file_;
    QTextStream csv_;
    QProgressDialog* exportProgressDlg_;
    WalletModel* transactionsModel_;

    void finish();
    void fetched();
    void writeHeader();
    void writeData();
    void close();
    void reset();
};

}
