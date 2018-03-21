// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QAbstractButton>
#include "sendconfirmationdialog.h"

namespace WalletGUI
{

SendConfirmationDialog::SendConfirmationDialog(
        const QString& title,
        const QString& text,
        int secDelay,
        QWidget *parent)
    : QMessageBox(QMessageBox::Question, title, text, QMessageBox::Yes | QMessageBox::Cancel, parent)
    , yesButton_(button(QMessageBox::Yes))
    , seconds_(secDelay)
{
    setDefaultButton(QMessageBox::Cancel);
    updateYesButton();
    connect(&countDownTimer_, &QTimer::timeout, this, &SendConfirmationDialog::countDown);
}

int SendConfirmationDialog::exec()
{
    updateYesButton();
    countDownTimer_.start(1000); // count down every 1 sec
    return QMessageBox::exec();
}

void SendConfirmationDialog::countDown()
{
    --seconds_;
    updateYesButton();

    if(seconds_ <= 0)
        countDownTimer_.stop();
}

void SendConfirmationDialog::updateYesButton()
{
    static int width = 0;
    if(seconds_ > 0)
    {
        yesButton_->setEnabled(false);
        yesButton_->setText(tr("Yes") + " (" + QString::number(seconds_) + ")");
        width = qMax(width, yesButton_->width());
        yesButton_->resize(width, yesButton_->height());
    }
    else
    {
        yesButton_->setEnabled(true);
        yesButton_->setText(tr("Yes"));
    }
}

}
