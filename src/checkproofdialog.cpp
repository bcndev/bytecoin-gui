// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QClipboard>
#include <QJsonObject>
#include <QJsonParseError>

#include "checkproofdialog.h"
#include "ui_checkproofdialog.h"
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
    ui->resultLabel->setText(/*QString("<b><font color='red'>%1</font></b>").arg(*/tr("Ready to check"))/*)*/;

    clear();

//    const QString proofString = ui->proofEdit->toPlainText();
//    const QByteArray proofBytes = proofString.toLatin1();

//    QJsonParseError parseError;
//    QJsonDocument jsonDocument = QJsonDocument::fromJson(proofBytes, &parseError);
//    if (parseError.error != QJsonParseError::NoError)
//    {
//        ui->resultLabel->setText(QString("<b><font color='red'>%1</font></b>").arg(tr("The entered text is not a valid JSON object")));
//        return;
//    }

//    if (!jsonDocument.isObject())
//    {
//        ui->resultLabel->setText(QString("<b><font color='red'>%1</font></b>").arg(tr("The entered text is not a valid JSON object")));
//        return;
//    }
//    const QJsonObject json = jsonDocument.object();
//    const RpcApi::Proof proof = RpcApi::Proof::fromJson(json.toVariantMap());

//    ui->messageLabel->setText(proof.message);
//    ui->amountLabel->setText(formatAmount(proof.amount) + " BCN");
//    ui->addressLabel->setText(proof.address);
//    ui->txHashLabel->setText(proof.transaction_hash);
}

void CheckProofDialog::checkProof()
{
    emit checkProofSignal(ui->proofEdit->toPlainText());
}

void CheckProofDialog::showCheckResult(const RpcApi::ProofCheck& result)
{
    if (!result.validation_error.isEmpty())
        ui->resultLabel->setText(QString("<b><font color='red'>%1</font></b>").arg(result.validation_error));
    else
    {
        ui->resultLabel->setText(QString("<b><font color='green'>%1</font></b>").arg(tr("The proof is correct!")));
        ui->messageLabel->setText(result.message);
        ui->amountLabel->setText(formatAmount(result.amount));
        ui->addressLabel->setText(result.address);
        ui->txHashLabel->setText(result.transaction_hash);
    }
}

}
