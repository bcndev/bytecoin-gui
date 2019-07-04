// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QMetaMethod>
#include <QUrl>
#include <QUrlQuery>
#include <QStyle>

#include "sendframe.h"
#include "transferframe.h"
#include "walletmodel.h"
#include "mainwindow.h"
#include "common.h"
#include "rpcapi.h"
#include "addressbookmanager.h"

#include "ui_sendframe.h"

namespace WalletGUI
{

namespace {

//const char SEND_FRAME_STYLE_SHEET[] =
//  "WalletGui--SendFrame {"
//    "background-color: #ffffff;"
//    "border: none;"
//  "}"

//  "WalletGui--SendFrame #m_sendScrollarea {"
//    "background-color: %backgroundColorGray%;"
//    "border: none;"
//    "border-bottom: 1px solid %borderColor%;"
//  "}"

//  "WalletGui--SendFrame #m_sendScrollarea QScrollBar:vertical {"
//    "margin-top: 0px;"
//    "border-top: none;"
//  "}"

//  "WalletGui--SendFrame #scrollAreaWidgetContents {"
//    "background-color: #ffffff;"
//    "border: none;"
//  "}"

//  "WalletGui--SendFrame #m_sendFeeFrame {"
//    "border: none;"
//    "border-top: 1px solid %borderColor%;"
//    "border-bottom: 1px solid %borderColor%;"
//  "}";

//const char PAYMENT_URL_AMOUNT_TAG[] = "amount";
//const char PAYMENT_URL_PAYMENT_ID_TAG[] = "payment_id";
//const char PAYMENT_URL_MESSAGE_TAG[] = "message";
//const char PAYMENT_URL_LABEL_TAG[] = "label";
const char MIXIN_CRITICAL_COLOR[] = "#ea161f";
const char MIXIN_NORMAL_COLOR[] = "#fdce00";
const char MIXIN_GOOD_COLOR[] = "#6ca025";
const char MIXIN_SLIDER_STYLE_SHEET_TEMPLATE[] =
  "QSlider::sub-page:horizontal {"
      "background: %COLOR%;"
      "border: 1px solid %COLOR%;"
  "}";

bool isValidPaymentId(const QString& /*paymentIdString*/)
{
    return true;
}

}

SendFrame::SendFrame(QWidget* parent)
    : QFrame(parent)
    , m_ui(new Ui::SendFrame)
    , m_mainWindow(nullptr)
    , walletModel_(nullptr)
    , addressBookModel_(nullptr)
    , addressBookManager_(nullptr)
{
    m_ui->setupUi(this);
    m_ui->m_mixinSpin->setMaximum(MAX_MIXIN_VALUE);
    mixinValueChanged(m_ui->m_mixinSlider->value());
//    setStyleSheet(Settings::instance().getCurrentStyle().makeStyleSheet(SEND_FRAME_STYLE_SHEET));

    addRecipientClicked();
}

SendFrame::~SendFrame()
{
    m_transfers.clear();
}

void SendFrame::addRecipient(const QString& address, const QString& label)
{
    if (m_transfers.size() == 1 && m_transfers[0]->getAddress().isEmpty())
    {
        m_transfers[0]->setAddress(address);
        if (!label.isEmpty())
            m_transfers[0]->setLabel(label);
    }
    else
    {
        addRecipientClicked();
        m_ui->m_sendScrollarea->widget()->updateGeometry();
        m_ui->m_sendScrollarea->updateGeometry();
        m_transfers.last()->setAddress(address);
        if (!label.isEmpty())
            m_transfers.last()->setLabel(label);
    }
}

void SendFrame::setMainWindow(MainWindow* mainWindow)
{
    m_mainWindow = mainWindow;
    for (TransferFrame* transfer : m_transfers)
        transfer->setMainWindow(mainWindow);
}

void SendFrame::setAddressBookModel(QAbstractItemModel* model)
{
    addressBookModel_ = model;
    for (TransferFrame* transfer : m_transfers)
        transfer->setAddressBookModel(model);
}

void SendFrame::setAddressBookManager(AddressBookManager* manager)
{
    addressBookManager_ = manager;
    for (TransferFrame* transfer : m_transfers)
        transfer->setAddressBookManager(manager);
}

void SendFrame::setWalletModel(WalletModel *model)
{
    walletModel_ = model;
    connect(walletModel_, &WalletModel::dataChanged, this, &SendFrame::updateFee);
}

void SendFrame::updateFee(const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &roles)
{
    if (roles.contains(WalletModel::ROLE_RECOMMENDED_FEE_PER_BYTE))
        feeValueChanged(m_ui->m_feeSlider->value());
}

void SendFrame::addRecipientClicked()
{
    TransferFrame* newTransferFrame = new TransferFrame(m_ui->m_sendScrollarea->widget());
    newTransferFrame->setMainWindow(m_mainWindow);
    newTransferFrame->setAddressBookModel(addressBookModel_);
    newTransferFrame->setAddressBookManager(addressBookManager_);
    m_ui->m_transfersLayout->insertWidget(m_transfers.size(), newTransferFrame);
    m_transfers.append(newTransferFrame);
    m_transfers.first()->hideBorder();
    if (m_transfers.size() == 1)
         newTransferFrame->disableRemoveButton(true);
    else
        m_transfers[0]->disableRemoveButton(false);

    connect(newTransferFrame, &TransferFrame::amountStringChangedSignal, this, &SendFrame::amountStringChanged);
    connect(newTransferFrame, &TransferFrame::addressChangedSignal, this, &SendFrame::addressChanged);
    connect(newTransferFrame, &TransferFrame::destroyed, [this, newTransferFrame](QObject* obj) {
        if (m_transfers.isEmpty())
            return;

        Q_ASSERT(newTransferFrame == obj);
        m_transfers.removeOne(/*static_cast<TransferFrame*>(obj)*/newTransferFrame);

        if (m_transfers.size() == 1)
            m_transfers[0]->disableRemoveButton(true);

        m_transfers.first()->hideBorder();
        m_ui->m_transfersLayout->removeWidget(newTransferFrame);
        m_ui->m_sendScrollarea->widget()->updateGeometry();
        m_ui->m_sendScrollarea->updateGeometry();
        amountStringChanged(QString());
        m_ui->m_sendButton->setEnabled(readyToSend());
    });

    amountStringChanged(QString());
    m_ui->m_sendScrollarea->widget()->adjustSize();
    m_ui->m_sendScrollarea->widget()->updateGeometry();
    m_ui->m_sendScrollarea->updateGeometry();
    m_ui->m_sendButton->setEnabled(false);
}

void SendFrame::clearAll()
{
    for (TransferFrame* transfer : m_transfers)
    {
        m_ui->m_transfersLayout->removeWidget(transfer);
        transfer->close();
        transfer->deleteLater();
    }

    QApplication::processEvents();
    m_transfers.clear();
//    m_ui->m_feeSpin->setValue(m_ui->m_feeSpin->minimum());
    m_ui->m_paymentIdEdit->clear();
    m_ui->m_mixinSlider->setValue(DEFAULT_MIXIN_VALUE);
    m_ui->m_sendScrollarea->widget()->adjustSize();
    m_ui->m_sendScrollarea->widget()->updateGeometry();
    m_ui->m_sendScrollarea->updateGeometry();

    m_ui->m_sendButton->setText(tr("Send"));
    addRecipientClicked();
}

void SendFrame::cancelSend()
{
    m_ui->m_sendButton->setText(tr("Send"));
    m_ui->m_sendButton->setEnabled(readyToSend());
}

void SendFrame::sendClicked()
{
//    Q_ASSERT(m_mainWindow != nullptr);
    Q_ASSERT(walletModel_ != nullptr);

    m_ui->m_sendButton->setEnabled(false);
    m_ui->m_sendButton->setText(tr("Checking..."));

    quint64 transferSum = 0;
    QList<RpcApi::Transfer> trs;
    for (TransferFrame* transfer : m_transfers)
    {
        const QString address = transfer->getAddress();
        const QString label = transfer->getLabel();
        if (!label.isEmpty() && addressBookManager_->findAddressByAddress(address) == INVALID_ADDRESS_INDEX &&
                                addressBookManager_->findAddressByLabel(label) == INVALID_ADDRESS_INDEX)
            addressBookManager_->addAddress(label, address);

//        const double damount = transfer->getAmountString().toDouble();
        qint64 amount = 0;
        if (!parseAmount(transfer->getAmountString(), amount))
            return;
        if (amount <= 0)
        {
            transfer->setAmountFormatError(true);
            m_ui->m_sendScrollarea->ensureWidgetVisible(transfer);
            m_ui->m_sendButton->setEnabled(readyToSend());
            m_ui->m_sendButton->setText(tr("Send"));
            return;
        }
        transferSum += amount;

        RpcApi::Transfer tr;
        tr.address = address;
        tr.amount = amount;
        trs << tr;
    }
    const QString paymentId = m_ui->m_paymentIdEdit->text();

    RpcApi::Transaction tx;
    tx.transfers = trs;
    tx.payment_id = paymentId;
    tx.anonymity = m_ui->m_mixinSlider->value();
    tx.unlock_block_or_timestamp = 0;

    emit createTxSignal(tx, getFeeFromSlider(m_ui->m_feeSlider->value()), m_ui->m_subtractFeeFromAmountCheckBox->isChecked());
}

void SendFrame::mixinValueChanged(int value)
{
    if (m_ui->m_mixinSlider->value() != value)
        m_ui->m_mixinSlider->setValue(value);

    if (m_ui->m_mixinSpin->value() != value)
        m_ui->m_mixinSpin->setValue(value);

    setMixinError(false);
    updateMixinSliderStyleSheet();
}

void SendFrame::feeValueChanged(int value)
{
    const quint64 fee = getFeeFromSlider(value);
    m_ui->m_feeTextLabel->setText(tr("Fee: %1 AU per byte").arg(fee));
}

quint64 SendFrame::getFeeFromSlider(int sliderValue) const
{
    Q_ASSERT(walletModel_ != nullptr);

    const quint64 recommendedFee = walletModel_->getRecommendedFeePerByte();
    quint64 fee = 0;
    switch(sliderValue)
    {
    case 0: fee = 0;                    break; // 0%
    case 1: fee = recommendedFee * 1/2; break; // 50%
    case 2: fee = recommendedFee;       break; // 100%
    case 3: fee = recommendedFee * 3/2; break; // 150%
    case 4: fee = recommendedFee * 2;   break; // 200%
    default: fee = recommendedFee; break;    // 100%
    }
    return fee;
}

void SendFrame::validatePaymentId(const QString& paymentId)
{
    bool validPaymentId = isValidPaymentId(paymentId);
    if (!validPaymentId)
    {
        setPaymentIdError(true);
        m_ui->m_sendButton->setEnabled(false);
        return;
    }

    setPaymentIdError(false);
    m_ui->m_sendButton->setEnabled(readyToSend());
}

void SendFrame::setPaymentIdError(bool error)
{
    m_ui->m_paymentIdEdit->setProperty("errorState", error);
    m_ui->m_paymentIdTextLabel->setProperty("errorState", error);
    if (error)
        m_ui->m_paymentIdTextLabel->setText(tr("INVALID FORMAT"));
    else
        m_ui->m_paymentIdTextLabel->setText(tr("Payment ID"));

    m_ui->m_paymentIdEdit->style()->unpolish(m_ui->m_paymentIdEdit);
    m_ui->m_paymentIdEdit->style()->polish(m_ui->m_paymentIdEdit);

    m_ui->m_paymentIdTextLabel->style()->unpolish(m_ui->m_paymentIdTextLabel);
    m_ui->m_paymentIdTextLabel->style()->polish(m_ui->m_paymentIdTextLabel);
    m_ui->m_paymentIdTextLabel->update();
}

void SendFrame::setMixinError(bool error)
{
    m_ui->m_mixinTextLabel->setProperty("errorState", error);
    if (error)
        m_ui->m_mixinTextLabel->setText(tr("MIXIN IS TOO BIG"));
    else
        m_ui->m_mixinTextLabel->setText(tr("Anonymity level"));

    m_ui->m_mixinTextLabel->style()->unpolish(m_ui->m_mixinTextLabel);
    m_ui->m_mixinTextLabel->style()->polish(m_ui->m_mixinTextLabel);
    m_ui->m_mixinTextLabel->update();
}

void SendFrame::setFeeFormatError(bool /*error*/)
{
}

void SendFrame::updateMixinSliderStyleSheet()
{
    int mixinValue = m_ui->m_mixinSpin->value();
    QString color = MIXIN_GOOD_COLOR;
    if (mixinValue < CRITICAL_MIXIN_BOUND)
        color = MIXIN_CRITICAL_COLOR;
    else if (mixinValue < NORMAL_MIXIN_BOUND)
        color = MIXIN_NORMAL_COLOR;

    QString mixinSliderStyleSheet = QString(MIXIN_SLIDER_STYLE_SHEET_TEMPLATE).replace("%COLOR%", color);
    m_ui->m_mixinSlider->setStyleSheet(mixinSliderStyleSheet);
}

void SendFrame::amountStringChanged(const QString& /*amountString*/)
{
    quint64 amount = 0;
    for (TransferFrame* transfer : m_transfers)
        amount += transfer->getAmount();

//    m_ui->m_totalAmountLabel->setText(QString("%1 %2")
//            .arg(formatUnsignedAmount(amount, false /*trim*/))
//            .arg(QString(CURRENCY_TICKER).toUpper()));

    m_ui->m_totalAmountLabel->setText(formatUnsignedAmount(amount, false /*trim*/));
    m_ui->m_sendButton->setEnabled(readyToSend());
}

void SendFrame::addressChanged(const QString& /*address*/)
{
    m_ui->m_sendButton->setEnabled(readyToSend());
}

bool SendFrame::readyToSend() const
{
    bool readyToSend = true;
    for (const auto& transfer : m_transfers)
        readyToSend = readyToSend && transfer->readyToSend();

    readyToSend = readyToSend && isValidPaymentId(m_ui->m_paymentIdEdit->text());
    return readyToSend;
}

}
