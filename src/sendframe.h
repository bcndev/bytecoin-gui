// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QFrame>
#include <QScrollArea>

class QAbstractItemModel;

namespace Ui {
class SendFrame;
}

namespace RpcApi {
struct Transaction;
}

namespace WalletGUI {

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
    void setWalletModel(WalletModel* model);
    void setMainWindow(MainWindow* mainWindow);
    void setAddressBookModel(QAbstractItemModel* model);
    void setAddressBookManager(AddressBookManager* manager);

    Q_SLOT void clearAll();
    Q_SLOT void cancelSend();

private:
    QScopedPointer<Ui::SendFrame> m_ui;
    MainWindow* m_mainWindow;
    QList<TransferFrame*> m_transfers;
    WalletModel* walletModel_;
    QAbstractItemModel* addressBookModel_;
    AddressBookManager* addressBookManager_;

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
    void createTxSignal(const RpcApi::Transaction& tx, quint64 fee, bool subtractFee);
};

}
