// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QFrame>
#include <QAbstractItemModel>

class QCompleter;

namespace Ui {
  class TransferFrame;
}

namespace WalletGUI
{

class AddressBookManager;

class TransferFrame : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(TransferFrame)

public:
    explicit TransferFrame(QWidget* parent);
    ~TransferFrame();

    bool readyToSend() const;
    QString getAddress() const;
    QString getAmountString() const;
    quint64 getAmount() const;
    QString getLabel() const;

    void setMainWindow(QWidget* mainWindow);
    void setAddressBookModel(QAbstractItemModel* model);
    void setAddressBookManager(AddressBookManager* manager);

    void setAddress(const QString& address);
    void setAmount(qreal amount);
    void setLabel(const QString& label);

    void disableRemoveButton(bool disable);
    void hideBorder();

    void setAddressError(bool error = true);
    void setDuplicationError(bool error = true);
    void setAmountFormatError(bool error = true);
    void setInsufficientFundsError(bool error = true);
    void setBigTransactionError(bool error = true);
    void setLabelLikePaymentIdError(bool error = true);

    virtual bool eventFilter(QObject* object, QEvent* event) override;

private:
    QScopedPointer<Ui::TransferFrame> m_ui;
    QWidget* m_mainWindow;
    QAbstractItemModel* m_addressBookModel;
    AddressBookManager* m_addressBookManager;
    QCompleter* m_addressCompleter;

    Q_SLOT void validateAmount(double amount);
    Q_SLOT void amountStringChanged(const QString& amountString);
    Q_SLOT void addressChanged(const QString& address);
    Q_SLOT void labelOrAddressChanged(const QString& text);
    Q_SLOT void addressBookClicked();
    Q_SLOT void pasteClicked();

Q_SIGNALS:
    void amountStringChangedSignal(const QString& amountString);
    void addressChangedSignal(const QString& address);
};

}
