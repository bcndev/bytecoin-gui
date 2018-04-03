// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QFileDialog>
#include <QMessageBox>

#include "importkeydialog.h"
#include "ui_importkeydialog.h"

namespace WalletGUI
{

ImportKeyDialog::ImportKeyDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ImportKeyDialog)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

ImportKeyDialog::~ImportKeyDialog()
{
    ui->keyEdit->setText(QString{1000, '0'});
    ui->keyEdit->clear();
    delete ui;
}

QByteArray ImportKeyDialog::getKey() const
{
    return QByteArray::fromHex(ui->keyEdit->toPlainText().toUtf8());
}

void ImportKeyDialog::loadKey()
{
    const QString filePath =
            QFileDialog::getOpenFileName(
                this,
                tr("Load key from..."),
                QDir::homePath(),
                tr("Key file (*)"));

    if (filePath.isEmpty())
        return;

    QFile keyFile(filePath);
    if (!keyFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to read key from the selected file."));
        return;
    }

    key_ = keyFile.readAll();
    keyFile.close();
    ui->keyEdit->setPlainText(key_.toHex().toUpper());
}

void ImportKeyDialog::keyChanged()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->keyEdit->toPlainText().length() == 256);
}

}
