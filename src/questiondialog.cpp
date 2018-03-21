// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include "questiondialog.h"

#include "ui_questiondialog.h"

namespace WalletGUI
{

QuestionDialog::QuestionDialog(const QString& title, const QString& text, QWidget* parent)
    : QDialog(parent, static_cast<Qt::WindowFlags>(Qt::WindowCloseButtonHint))
    , ui(new Ui::QuestionDialog)
{
    ui->setupUi(this);
    setWindowTitle(title);
    ui->questionLabel->setText(text);
}

QuestionDialog::~QuestionDialog()
{}

}
