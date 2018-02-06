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

#include <QFrame>
#include <QScrollArea>

//#include "Application/IWalletUiItem.h"
//#include "IAddressBookManager.h"
//#include "IApplicationEventHandler.h"
//#include "ICryptoNoteAdapter.h"
//#include "IWalletAdapter.h"

class QAbstractItemModel;

namespace Ui {
class SendFrame;
}

namespace RpcApi {
struct Transaction;
}

namespace WalletGUI {

//class ICryptoNoteAdapter;
//class SendGlassFrame;
class TransferFrame;
class WalletModel;
class MainWindow;
class AddressBookManager;

class SendFrame : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(SendFrame)

public:
    explicit SendFrame(QWidget* parent);
    ~SendFrame();

    void addRecipient(const QString& address, const QString& label = QString());
    //  void setMainWindow(QWidget* mainWindow);
    void setWalletModel(WalletModel* model);
    void setMainWindow(MainWindow* mainWindow);
    void setAddressBookModel(QAbstractItemModel* model);
    void setAddressBookManager(AddressBookManager* manager);

    Q_SLOT void clearAll();
    Q_SLOT void cancelSend();

private:
    QScopedPointer<Ui::SendFrame> m_ui;
    //  QWidget* m_mainWindow;
    MainWindow* m_mainWindow;
    QList<TransferFrame*> m_transfers;
    WalletModel* walletModel_;
    QAbstractItemModel* addressBookModel_;
    AddressBookManager* addressBookManager_;

    //  void processTranactionSendStatus(IWalletAdapter::SendTransactionStatus status);
    void setPaymentIdError(bool error);
    void setMixinError(bool error);
    void setFeeFormatError(bool error);
    void updateMixinSliderStyleSheet();
    void amountStringChanged(const QString& amountString);
    void addressChanged(const QString& address);
    bool readyToSend() const;
    quint64 getFeeFromSlider(int sliderValue) const;

    Q_SLOT void addRecipientClicked();
    Q_SLOT void sendClicked();
    Q_SLOT void mixinValueChanged(int value);
    Q_SLOT void feeValueChanged(int value);
    Q_SLOT void validatePaymentId(const QString& paymentId);
    Q_SLOT void updateFee(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);

Q_SIGNALS:
    void showTransactionsFrameSignal();
    void createTxSignal(const RpcApi::Transaction& tx, quint64 fee);
};

}
