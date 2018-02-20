#include <QFileDialog>

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
    return QByteArray::fromHex(ui->keyEdit->toPlainText().toLatin1());
}

void ImportKeyDialog::loadKey()
{
    const QString filePath =
            QFileDialog::getOpenFileName(
                this,
                tr("Load key from..."),
                QDir::homePath(),
                tr("Key file (*.*)"));

    if (filePath.isEmpty())
        return;

    QFile keyFile(filePath);
    if (!keyFile.open(QIODevice::ReadOnly))
        return;

    key_ = keyFile.readAll();
    keyFile.close();
    ui->keyEdit->setPlainText(key_.toHex().toUpper());
}

void ImportKeyDialog::keyChanged()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ui->keyEdit->toPlainText().length() == 256);
}

}
