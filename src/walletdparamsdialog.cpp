// Copyright (c) 2015-2018, The Armor developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include "walletdparamsdialog.h"
#include "ui_walletdparamsdialog.h"
#include "settings.h"

namespace WalletGUI
{

WalletdParamsDialog::WalletdParamsDialog(bool allowToRestart, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WalletdParamsDialog)
    , allowToRestart_(allowToRestart)
{
    ui->setupUi(this);
//    ui->applyButton->setEnabled(allowToRestart);
    ui->applyButton->setText(allowToRestart_ ? tr("Save and restart walletd") : tr("Save"));
    ui->paramsEdit->setText(Settings::instance().getWalletdParams());

    const NetworkType net = Settings::instance().getNetworkType();
    switch (net) {
    case NetworkType::MAIN:  ui->radioMainNet->setChecked(true);  break;
    case NetworkType::STAGE: ui->radioStageNet->setChecked(true); break;
    case NetworkType::TEST:  ui->radioTestNet->setChecked(true);  break;
    default: ui->radioMainNet->setChecked(true);  break;
    }

    const ConnectionMethod armordConnection = Settings::instance().getArmordConnectionMethod();
    switch (armordConnection) {
    case ConnectionMethod::BUILTIN:  ui->radioBuiltinArmord->setChecked(true);  break;
    case ConnectionMethod::REMOTE: ui->radioExternalArmord->setChecked(true); break;
    default: ui->radioBuiltinArmord->setChecked(true);  break;
    }

    QUrl url{Settings::instance().getArmordHost()};
    url.setPort(Settings::instance().getArmordPort());
    ui->editHost->setText(url.toString());
}

WalletdParamsDialog::~WalletdParamsDialog()
{
    delete ui;
}

void WalletdParamsDialog::saveParams()
{
    Settings::instance().setWalletdParams(ui->paramsEdit->toPlainText().simplified());
    Settings::instance().setNetworkType(
                ui->radioMainNet->isChecked() ?
                    NetworkType::MAIN :
                    ui->radioStageNet->isChecked() ?
                        NetworkType::STAGE :
                        ui->radioTestNet->isChecked() ?
                            NetworkType::TEST :
                            NetworkType::MAIN);
    Settings::instance().setArmordConnectionMethod(
                ui->radioBuiltinArmord->isChecked() ?
                    ConnectionMethod::BUILTIN :
                    ui->radioExternalArmord->isChecked() ?
                        ConnectionMethod::REMOTE :
                        ConnectionMethod::BUILTIN);

    const QUrl url{ui->editHost->text()};
    Settings::instance().setArmordEndPoint(url.toString(QUrl::RemovePort), url.port(Settings::instance().getDefaultArmordPort()));
}

void WalletdParamsDialog::applyParams()
{
    saveParams();
    if (allowToRestart_)
        emit restartWalletd();
}

void WalletdParamsDialog::hostChanged()
{
    ui->applyButton->setEnabled(!ui->editHost->text().isEmpty());
}

void WalletdParamsDialog::externalArmordToggled(bool checked)
{
    if (checked)
        ui->applyButton->setEnabled(!ui->editHost->text().isEmpty());
    else
        ui->applyButton->setEnabled(true);
}

}
