// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include "crashdialog.h"
#include "ui_crashdialog.h"
#include "mainwindow.h"

namespace WalletGUI {

CrashDialog::CrashDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CrashDialog)
    , mainWindow_(nullptr)
{
    ui->setupUi(this);
}

CrashDialog::~CrashDialog()
{
    delete ui;
}

void CrashDialog::enablePasswordEdit(bool enable)
{
    ui->passwordLabel->setVisible(enable);
    ui->passwordLineEdit->setVisible(enable);
    ui->passwordLineEdit->setText("");
}

int CrashDialog::execWithReason(const QString& reason, bool enablePassword)
{
    enablePasswordEdit(enablePassword);
    ui->reasonLabel->setText(reason);
    return QDialog::exec();
}

QString CrashDialog::getPassword() const
{
    return ui->passwordLineEdit->text();
}

void CrashDialog::showDaemonConsole()
{
    if (mainWindow_)
        mainWindow_->showLog();
}

void CrashDialog::setMainWindow(MainWindow* mainWindow)
{
    mainWindow_ = mainWindow;
}

} // namespace WalletGUI
