// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include "mnemonicdialog.h"
#include "ui_mnemonicdialog.h"
#include "walletd.h"

namespace WalletGUI
{

MnemonicDialog::MnemonicDialog(bool generate, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MnemonicDialog)
    , parent_(parent)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    ui->mnemonicEdit->setReadOnly(generate);
    ui->generateButton->setEnabled(generate);
    ui->generateButton->setVisible(generate);
    ui->descriptionLabel->setVisible(generate);
    if (generate)
        generateMnemonic();
}
MnemonicDialog::~MnemonicDialog()
{
    ui->mnemonicEdit->setText(QString{1000, '0'});
    ui->mnemonicEdit->clear();
    delete ui;
}

QByteArray MnemonicDialog::getMnemonic() const
{
    return ui->mnemonicEdit->toPlainText().simplified().toUtf8();
}

void MnemonicDialog::mnemonicChanged()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!ui->mnemonicEdit->toPlainText().isEmpty());
}

void MnemonicDialog::generateMnemonic()
{
    ui->mnemonicEdit->setText(BuiltinWalletd::generateMnemonic(parent_, std::bind(&MnemonicDialog::daemonStandardErrorReady, this, std::placeholders::_1)).simplified());
}

void MnemonicDialog::daemonStandardErrorReady(QString err)
{
    ui->mnemonicEdit->setText(QString{"<b><font color='red'>%1</font></b>"}.arg(err));
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}


}

