// Copyright (c) 2015-2017, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <QMainWindow>
#include <QProcess>

#include "rpcapi.h"

class QActionGroup;
class QDataWidgetMapper;
class QSplashScreen;
class QAbstractButton;
class QAbstractItemModel;
class QSessionManager;

namespace Ui {
  class MainWindow;
}

namespace WalletGUI {

class WalletModel;
class MiningManager;
class AddressBookManager;
class CopiedToolTip;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

public:
    MainWindow(
            WalletModel* walletModel,
            const QString& styleSheetTemplate,
            MiningManager* miningManager,
            AddressBookManager* addressBookManager,
            QWidget* parent);
    virtual ~MainWindow();

    bool eventFilter(QObject* object, QEvent* event) override;

    void addRecipient(const QString& address, const QString& label = QString());
    QString getAddress() const;

    Q_SLOT void setConnectedState();
    Q_SLOT void setDisconnectedState();

    Q_SLOT void builtinRun();
//    Q_SLOT void remoteConnected();

    Q_SLOT void jsonErrorResponse(const QString& id, const QString& errorString);
    Q_SLOT void createTxReceived(const RpcApi::CreatedTx& tx);
    Q_SLOT void createTx(const RpcApi::Transaction& tx, quint64 fee);

    Q_SLOT void copiedToClipboard();
    Q_SLOT void openDataFolder();
    Q_SLOT void splashMsg(const QString& msg);
    Q_SLOT void addDaemonOutput(const QString& msg);
    Q_SLOT void addDaemonError(const QString& msg);
    Q_SLOT void showLog();

    Q_SLOT void packetSent(const QByteArray& data);
    Q_SLOT void packetReceived(const QByteArray& data);
    Q_SLOT void importKeys();

protected:
    void changeEvent(QEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    QScopedPointer<Ui::MainWindow> m_ui;
    QAbstractItemModel* m_addressBookModel;
    QAbstractItemModel* m_sortedAddressBookModel;
    QAbstractItemModel* m_minerModel;
    QString m_styleSheetTemplate;
    QDataWidgetMapper* m_addressesMapper;
    QDataWidgetMapper* m_balanceMapper;
    QMovie* m_syncMovie;
    MiningManager* m_miningManager;
    AddressBookManager* addressBookManager_;
    CopiedToolTip* copiedToolTip_;

    WalletModel* walletModel_;

    void createRecentWalletMenu();
    void updateRecentWalletActions();
    void themeChanged();
    void setOpenedState();
    void setClosedState();
    void walletStateModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void showSendConfirmation(const RpcApi::CreatedTx& tx);
    void setTitle();
    void clearTitle();

    Q_SLOT void aboutQt();
    Q_SLOT void about();
    Q_SLOT void copyAddress();
    Q_SLOT void copyBalance();
    Q_SLOT void communityForumTriggered();
    Q_SLOT void reportIssueTriggered();

    Q_SLOT void createWallet();
    Q_SLOT void openWallet();
    Q_SLOT void remoteWallet();
    Q_SLOT void encryptWallet();

signals:
    void createTxSignal(const RpcApi::CreateTransaction::Request& req, QPrivateSignal);
    void sendTxSignal(const RpcApi::SendTransaction::Request& req, QPrivateSignal);
    void restartDaemon(QPrivateSignal);

    void createWalletSignal(QWidget* parent);
    void openWalletSignal(QWidget* parent);
    void remoteWalletSignal(QWidget* parent);
    void encryptWalletSignal(QWidget* parent);
    void importKeysSignal(QWidget* parent);
};

}
