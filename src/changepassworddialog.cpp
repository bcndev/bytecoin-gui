// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QPushButton>

#include "changepassworddialog.h"
#include "ui_changepassworddialog.h"

namespace WalletGUI {

ChangePasswordDialog::ChangePasswordDialog(bool askCurrentPassword, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChangePasswordDialog)
    , oldPassword_("")
    , newPassword_("")
    , confirmation_("")
{
    ui->setupUi(this);

    ui->currPasswordLabel->setVisible(askCurrentPassword);
    ui->currPasswordEdit->setVisible(askCurrentPassword);
    adjustSize();
}

ChangePasswordDialog::~ChangePasswordDialog()
{
    oldPassword_.fill('0', 200);
    newPassword_.fill('0', 200);
    confirmation_.fill('0', 200);
    oldPassword_.clear();
    newPassword_.clear();
    confirmation_.clear();

    delete ui;
}

void ChangePasswordDialog::passwordChanged(const QString& password)
{
    newPassword_ = password;
    updateOkButton();
}

void ChangePasswordDialog::confirmationChanged(const QString& confirmation)
{
    confirmation_ = confirmation;
    updateOkButton();
}

void ChangePasswordDialog::updateOkButton()
{
    const bool ok = (newPassword_ == confirmation_);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ok);
}

QString ChangePasswordDialog::getOldPassword() const
{
    return ui->currPasswordEdit->text();
}

QString ChangePasswordDialog::getNewPassword() const
{
    return newPassword_;
}

} // namespace WalletGUI
