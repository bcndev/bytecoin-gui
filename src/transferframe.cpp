// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QClipboard>
#include <QCompleter>
#include <QWheelEvent>
#include <QStyle>

#include "transferframe.h"
//#include "Settings/Settings.h"
//#include "Common/AddressBookDialog.h"
//#include "ICryptoNoteAdapter.h"
//#include "Models/AddressBookModel.h"
//#include "Style/Style.h"
#include "addressbookmodel.h"
#include "addressbookdialog.h"
#include "addressbookmanager.h"
#include "common.h"

#include "ui_transferframe.h"

namespace WalletGUI {

namespace {

//const char TRANSFER_FRAME_STYLE_SHEET_TEMPLATE[] =
//  "WalletGui--TransferFrame {"
//    "background-color: %backgroundColorGray%;"
//    "border: none;"
//    "border-top: 1px solid %borderColor%;"
//  "}"

//  "WalletGui--TransferFrame[hideTopBorder=\"true\"] {"
//    "border-top: none"
//  "}"

//  "WalletGui--TransferFrame #m_addressBookButton {"
//    "margin-top: 1px;"
//  "}"

//  "WalletGui--TransferFrame #m_sendAmountSpin {"
//    "min-width: 230px;"
//    "max-width: 230px;"
//  "}";

}

TransferFrame::TransferFrame(QWidget* parent)
    : QFrame(parent)
    , m_ui(new Ui::TransferFrame)
    , m_mainWindow(nullptr)
    , m_addressBookModel(nullptr)
    , m_addressBookManager(nullptr)
    , m_addressCompleter(new QCompleter(this))
{
    m_ui->setupUi(this);
    m_ui->m_sendAmountSpin->installEventFilter(this);
//    setStyleSheet(Settings::instance().getCurrentStyle().makeStyleSheet(TRANSFER_FRAME_STYLE_SHEET_TEMPLATE));
}

TransferFrame::~TransferFrame()
{}

bool TransferFrame::readyToSend() const
{
    const QString address = getAddress();
    const double amount = m_ui->m_sendAmountSpin->value();
    return !address.isEmpty() && amount > 0/* && m_cryptoNoteAdapter->isValidAddress(address)*/;
}

QString TransferFrame::getAddress() const
{
    return m_ui->m_sendAddressEdit->text().trimmed();
}

QString TransferFrame::getAmountString() const
{
    return m_ui->m_sendAmountSpin->cleanText();
}

quint64 TransferFrame::getAmount() const
{
    qint64 amount  = 0;
    const bool ok = parseAmount(getAmountString(), amount);
    Q_ASSERT(ok);
    Q_ASSERT(amount >= 0);
    return static_cast<quint64>(amount);
}

QString TransferFrame::getLabel() const
{
    return m_ui->m_sendLabelEdit->text().trimmed();
}

void TransferFrame::setAddress(const QString& address)
{
    m_ui->m_sendAddressEdit->setText(address);
}

void TransferFrame::setAmount(qreal amount)
{
    m_ui->m_sendAmountSpin->setValue(amount);
}

void TransferFrame::setLabel(const QString& label)
{
    m_ui->m_sendLabelEdit->setText(label);
}

void TransferFrame::disableRemoveButton(bool disable)
{
    m_ui->m_removeButton->setHidden(disable);
}

void TransferFrame::hideBorder()
{
    setProperty("hideTopBorder", true);
    style()->unpolish(this);
    style()->polish(this);
}

void TransferFrame::setAddressError(bool error)
{
    m_ui->m_payToTextLabel->setProperty("errorState", error);
    m_ui->m_sendAddressEdit->setProperty("errorState", error);
    if (error)
        m_ui->m_payToTextLabel->setText(tr("INVALID ADDRESS"));
    else
        m_ui->m_payToTextLabel->setText(tr("Pay to"));

    m_ui->m_sendAddressEdit->style()->unpolish(m_ui->m_sendAddressEdit);
    m_ui->m_sendAddressEdit->style()->polish(m_ui->m_sendAddressEdit);
    m_ui->m_sendAddressEdit->update();

    m_ui->m_payToTextLabel->style()->unpolish(m_ui->m_payToTextLabel);
    m_ui->m_payToTextLabel->style()->polish(m_ui->m_payToTextLabel);
    m_ui->m_payToTextLabel->update();
}

void TransferFrame::setDuplicationError(bool error)
{
    m_ui->m_sendLabelEdit->setProperty("errorState", error);
    m_ui->m_labelTextLabel->setProperty("errorState", error);
    m_ui->m_labelTextLabel->setText(error ?
            tr("ADDRESS OR LABEL ALREADY EXISTS IN THE ADDRESS BOOK AND WILL NOT BE ADDED") :
            tr("Label"));

    m_ui->m_sendLabelEdit->style()->unpolish(m_ui->m_sendLabelEdit);
    m_ui->m_sendLabelEdit->style()->polish(m_ui->m_sendLabelEdit);
    m_ui->m_sendLabelEdit->update();

    m_ui->m_labelTextLabel->style()->unpolish(m_ui->m_labelTextLabel);
    m_ui->m_labelTextLabel->style()->polish(m_ui->m_labelTextLabel);
    m_ui->m_labelTextLabel->update();
}

void TransferFrame::setAmountFormatError(bool error)
{
    m_ui->m_amountTextLabel->setProperty("errorState", error);
    m_ui->m_sendAmountSpin->setProperty("errorState", error);
    if (error)
    {
        m_ui->m_amountTextLabel->setText(tr("WRONG AMOUNT"));
        connect(m_ui->m_sendAmountSpin, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
                &TransferFrame::validateAmount, Qt::UniqueConnection);
    }
    else
        m_ui->m_amountTextLabel->setText(tr("Amount (BCN)"));

    m_ui->m_sendAmountSpin->style()->unpolish(m_ui->m_sendAmountSpin);
    m_ui->m_sendAmountSpin->style()->polish(m_ui->m_sendAmountSpin);
    m_ui->m_sendAmountSpin->update();

    m_ui->m_amountTextLabel->style()->unpolish(m_ui->m_amountTextLabel);
    m_ui->m_amountTextLabel->style()->polish(m_ui->m_amountTextLabel);
    m_ui->m_amountTextLabel->update();
}

void TransferFrame::setInsufficientFundsError(bool error)
{
    m_ui->m_amountTextLabel->setProperty("errorState", error);
    m_ui->m_sendAmountSpin->setProperty("errorState", error);
    if (error)
    {
        m_ui->m_amountTextLabel->setText(tr("INSUFFICIENT FUNDS"));
        connect(m_ui->m_sendAmountSpin, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
                &TransferFrame::validateAmount, Qt::UniqueConnection);
    }
    else
        m_ui->m_amountTextLabel->setText(tr("Amount (BCN)"));

    m_ui->m_sendAmountSpin->style()->unpolish(m_ui->m_sendAmountSpin);
    m_ui->m_sendAmountSpin->style()->polish(m_ui->m_sendAmountSpin);
    m_ui->m_sendAmountSpin->update();

    m_ui->m_amountTextLabel->style()->unpolish(m_ui->m_amountTextLabel);
    m_ui->m_amountTextLabel->style()->polish(m_ui->m_amountTextLabel);
    m_ui->m_amountTextLabel->update();
}

void TransferFrame::setBigTransactionError(bool error)
{
    m_ui->m_amountTextLabel->setProperty("errorState", error);
    m_ui->m_sendAmountSpin->setProperty("errorState", error);
    if (error)
    {
        m_ui->m_amountTextLabel->setText(tr("TRANSACTION IS TOO BIG"));
        connect(m_ui->m_sendAmountSpin, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this,
                &TransferFrame::validateAmount, Qt::UniqueConnection);
    }
    else
        m_ui->m_amountTextLabel->setText(tr("Amount (BCN)"));

    m_ui->m_sendAmountSpin->style()->unpolish(m_ui->m_sendAmountSpin);
    m_ui->m_sendAmountSpin->style()->polish(m_ui->m_sendAmountSpin);
    m_ui->m_sendAmountSpin->update();

    m_ui->m_amountTextLabel->style()->unpolish(m_ui->m_amountTextLabel);
    m_ui->m_amountTextLabel->style()->polish(m_ui->m_amountTextLabel);
    m_ui->m_amountTextLabel->update();
}

void TransferFrame::setLabelLikePaymentIdError(bool error)
{
    m_ui->m_sendLabelEdit->setProperty("errorState", error);
    m_ui->m_labelTextLabel->setProperty("errorState", error);
    m_ui->m_labelTextLabel->setText(error ? tr("NOTE: THIS IS NOT A PAYMENT ID FIELD") : tr("Label"));

    m_ui->m_sendLabelEdit->style()->unpolish(m_ui->m_sendLabelEdit);
    m_ui->m_sendLabelEdit->style()->polish(m_ui->m_sendLabelEdit);
    m_ui->m_sendLabelEdit->update();

    m_ui->m_labelTextLabel->style()->unpolish(m_ui->m_labelTextLabel);
    m_ui->m_labelTextLabel->style()->polish(m_ui->m_labelTextLabel);
    m_ui->m_labelTextLabel->update();
}

bool TransferFrame::eventFilter(QObject* object, QEvent* event) {
  if (object == m_ui->m_sendAmountSpin) {
    if (event->type() == QEvent::Wheel && !m_ui->m_sendAmountSpin->hasFocus()) {
      wheelEvent(static_cast<QWheelEvent*>(event));
      return true;
    }
  }

  return false;
}

void TransferFrame::setMainWindow(QWidget* mainWindow)
{
    m_mainWindow = mainWindow;
}

void TransferFrame::setAddressBookManager(AddressBookManager *manager)
{
    m_addressBookManager = manager;
}

void TransferFrame::setAddressBookModel(QAbstractItemModel* model)
{
    m_addressBookModel = model;
    m_addressCompleter->setModel(m_addressBookModel);
    m_addressCompleter->setCompletionColumn(AddressBookModel::COLUMN_LABEL);
    m_addressCompleter->setCompletionRole(AddressBookModel::ROLE_LABEL);
    m_addressCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    m_ui->m_sendAddressEdit->setCompleter(m_addressCompleter);

    connect(m_addressCompleter, static_cast<void(QCompleter::*)(const QModelIndex&)>(&QCompleter::activated), this,
        [&](const QModelIndex& index)
        {
            m_ui->m_sendAddressEdit->setText(index.data(AddressBookModel::ROLE_ADDRESS).toString());
        }, Qt::QueuedConnection);
}

void TransferFrame::addressBookClicked()
{
    AddressBookDialog dlg(m_addressBookModel, m_mainWindow);
    if (dlg.exec() == QDialog::Accepted)
    {
        m_ui->m_sendAddressEdit->setText(dlg.getAddress());
        m_ui->m_sendLabelEdit->setText(dlg.getLabel());
    }
}

void TransferFrame::pasteClicked()
{
    m_ui->m_sendAddressEdit->setText(QApplication::clipboard()->text());
}

void TransferFrame::addressChanged(const QString& address)
{
//    setAddressError(m_addressCompleter->currentCompletion().isEmpty() && !address.isEmpty() && !m_cryptoNoteAdapter->isValidAddress(address));
    setAddressError(address.isEmpty());
    Q_EMIT addressChangedSignal(address);
}

void TransferFrame::labelOrAddressChanged(const QString& /*text*/)
{
    QString label = getLabel().trimmed();
    QString address = getAddress().trimmed();
    if (!label.isEmpty() && (m_addressBookManager->findAddressByAddress(address) != INVALID_ADDRESS_INDEX ||
                             m_addressBookManager->findAddressByLabel(label) != INVALID_ADDRESS_INDEX))
    {
        setDuplicationError(true);
        return;
    }
//    else if (m_cryptoNoteAdapter->isValidPaymentId(label))
//    {
//        setLabelLikePaymentIdError(true);
//        return;
//    }

    setDuplicationError(false);
}

void TransferFrame::validateAmount(double amount)
{
    if (amount > 0)
        setAmountFormatError(false);
    else
        setAmountFormatError(true);
}

void TransferFrame::amountStringChanged(const QString& /*amountString*/)
{
    Q_EMIT amountStringChangedSignal(m_ui->m_sendAmountSpin->cleanText());
}

}
