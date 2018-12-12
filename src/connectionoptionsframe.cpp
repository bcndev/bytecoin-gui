// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QUrl>
#include <QFileDialog>

//#include <CryptoNoteConfig.h>

#include "connectionoptionsframe.h"
#include "settings.h"
#include "common.h"

#include "ui_connectionoptionsframe.h"

namespace WalletGUI {

namespace {

//const char CONNECTION_OPTIONS_STYLE_SHEET_TEMPLATE[] =
//  "WalletGui--ConnectionOptionsFrame {"
//    "background: #ffffff;"
//  "}"

//  "WalletGui--ConnectionOptionsFrame QRadioButton {"
//    "spacing: 20px;"
//  "}"

//  "WalletGui--ConnectionOptionsFrame QRadioButton,"
//  "WalletGui--ConnectionOptionsFrame QLineEdit,"
//  "WalletGui--ConnectionOptionsFrame QSpinBox {"
//    "min-height: 25px;"
//    "max-height: 25px;"
//  "}";

//const char VALIDATION_LABEL_STYLE_SHEET[] =
//    "WalletGUI--ConnectionOptionsFrame QLabel"
//    "{"
//        "color : red;"
//    "}";

enum class ErrorFlags
{
    NO_ERROR = 0,
    WALLET = 1,
    HOST = 2,
};

}

ConnectionOptionsFrame::ConnectionOptionsFrame(QWidget* parent)
    : QFrame(parent)
    , m_ui(new Ui::ConnectionOptionsFrame)
{
    m_ui->setupUi(this);
}

ConnectionOptionsFrame::~ConnectionOptionsFrame()
{}

void ConnectionOptionsFrame::load()
{
    const QUrl remoteUrl = QUrl::fromUserInput(Settings::instance().getRemoteWalletdEndPoint());
    if (!remoteUrl.isValid())
        m_ui->m_remotePortSpin->setValue(Settings::instance().getDefaultWalletdPort());
    else
    {
        m_ui->m_remoteHostEdit->setText(remoteUrl.host());
        m_ui->m_remotePortSpin->setValue(remoteUrl.port());
    }
    remoteHostNameChanged(m_ui->m_remoteHostEdit->text());
}

void ConnectionOptionsFrame::save()
{
    Settings::instance().setWalletdConnectionMethod(ConnectionMethod::REMOTE);
    Settings::instance().setRemoteWalletdEndPoint(m_ui->m_remoteHostEdit->text(), m_ui->m_remotePortSpin->value());

    switch (Settings::instance().getWalletdConnectionMethod())
    {
    case ConnectionMethod::BUILTIN:
        break;
    case ConnectionMethod::LOCAL:
    case ConnectionMethod::REMOTE:
        Settings::instance().addRecentWallet(Settings::instance().getWalletdEndPoint());
        break;
    }
}

bool ConnectionOptionsFrame::canAccept() const
{
    return isIpOrHostName(m_ui->m_remoteHostEdit->text());
}

bool ConnectionOptionsFrame::remoteHostNameChanged(const QString& host)
{
    bool valid = true;
    valid = isIpOrHostName(host);
//    const QString errMsg = valid ? QString() : tr("Invalid host name.");
    emit disableAcceptButtonSignal(!valid);
    return valid;
}

bool ConnectionOptionsFrame::walletFileNameChanged(const QString& /*wallet*/)
{
    return true;
}

void ConnectionOptionsFrame::connectionButtonClicked(int /*buttonId*/)
{
    remoteHostNameChanged(m_ui->m_remoteHostEdit->text());
}

void ConnectionOptionsFrame::showOpenWalletFileDialog()
{
    const QString fileName = QFileDialog::getOpenFileName(
                this,
                tr("Open wallet file"),
                QString(),
                tr("Wallet files (*.wallet);;All files (*)"));
    if (fileName.isEmpty())
        return;
}

void ConnectionOptionsFrame::showCreateWalletFileDialog()
{
    const QString fileName = QFileDialog::getSaveFileName(
                this,
                tr("Create wallet file"),
                QString(),
                tr("Wallet files (*.wallet);;All files (*)"));
    if (fileName.isEmpty())
        return;
}

}
