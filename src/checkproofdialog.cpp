// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QClipboard>
#include <QJsonObject>
#include <QJsonParseError>

#include "checkproofdialog.h"
#include "ui_checkproofdialog.h"
#include "rpcapi.h"
#include "common.h"

namespace WalletGUI
{

CheckProofDialog::CheckProofDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CheckProofDialog)
{
    ui->setupUi(this);
    clear();
}

CheckProofDialog::~CheckProofDialog()
{
    delete ui;
}

void CheckProofDialog::clear()
{
    ui->messageLabel->clear();
    ui->amountLabel->clear();
    ui->addressLabel->clear();
    ui->txHashLabel->clear();
}

void CheckProofDialog::pasteFromClipboard()
{
    ui->proofEdit->setPlainText(QApplication::clipboard()->text());
}

void CheckProofDialog::proofChanged()
{
    ui->resultLabel->setText(QString("<b><font color='red'>%1</font></b>").arg(tr("Unchecked")));

    clear();

    const QString proofString = ui->proofEdit->toPlainText();
    const QByteArray proofBytes = proofString.toLatin1();

    QJsonParseError parseError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(proofBytes, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return;

    if (!jsonDocument.isObject())
        return;
    const QJsonObject json = jsonDocument.object();
    const RpcApi::Proof proof = RpcApi::Proof::fromJson(json.toVariantMap());

    ui->messageLabel->setText(proof.message);
    ui->amountLabel->setText(formatAmount(proof.amount) + " BCN");
    ui->addressLabel->setText(proof.address);
    ui->txHashLabel->setText(proof.transaction_hash);
}

void CheckProofDialog::checkProof()
{
    emit checkProofSignal(ui->proofEdit->toPlainText());
}

void CheckProofDialog::showCheckResult(const QString& result)
{
    if (result.isEmpty())
        ui->resultLabel->setText(QString("<b><font color='green'>%1</font></b>").arg(tr("The proof is correct!")));
    else
        ui->resultLabel->setText(QString("<b><font color='red'>%1</font></b>").arg(result));
}

}
