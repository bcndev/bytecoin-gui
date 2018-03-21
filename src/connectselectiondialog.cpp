// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QPushButton>

#include "connectselectiondialog.h"
#include "ui_connectselectiondialog.h"

namespace WalletGUI
{

ConnectSelectionDialog::ConnectSelectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectSelectionDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)-> setEnabled(false);
    connect(ui->m_connectionOptionsFrame, &ConnectionOptionsFrame::disableAcceptButtonSignal,
        [this](bool disable)
        {
            ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!disable);
        });
    ui->m_connectionOptionsFrame->load();
}

ConnectSelectionDialog::~ConnectSelectionDialog()
{
    delete ui;
}

void ConnectSelectionDialog::accept()
{
    ui->m_connectionOptionsFrame->save();
    QDialog::accept();
}

}
