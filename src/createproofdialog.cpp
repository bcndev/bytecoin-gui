// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QJsonDocument>
#include <QJsonObject>
#include <QClipboard>

#include "createproofdialog.h"
#include "ui_createproofdialog.h"

namespace WalletGUI
{

CreateProofDialog::CreateProofDialog(const QString& txHash, const QStringList& addresses, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateProofDialog)
    , txHash_(txHash)
{
    ui->setupUi(this);
    ui->addressesBox->addItems(addresses);
//    showElements(false);
//    adjustSize();
//    ui->verticalLayout->update();
}

CreateProofDialog::~CreateProofDialog()
{
    delete ui;
}

//void CreateProofDialog::showElements(bool show)
//{
////    ui->addressLabel->setVisible(show);
////    ui->addressesBox->setVisible(show);
//    ui->proofLabel->setVisible(show);
//    ui->proofEdit->setVisible(show);
//    ui->copyButton->setVisible(show);
//}

//QString CreateProofDialog::extractAddress(const QString& proof)
//{
//    static constexpr char ADDRESS_KEY[] = "address";

//    QJsonParseError parseError;
//    QJsonDocument jsonDocument = QJsonDocument::fromJson(proof.toLatin1(), &parseError);
//    if (parseError.error != QJsonParseError::NoError)
//    {
//        ui->proofEdit->setTextColor(Qt::red);
//        ui->proofEdit->setText(parseError.errorString());
//        return QString{};
//    }

//    if (!jsonDocument.isObject())
//    {
//        ui->proofEdit->setTextColor(Qt::red);
//        ui->proofEdit->setText(tr("JSON document is not an object."));
//        return QString{};
//    }
//    const QJsonObject json = jsonDocument.object();
//    const bool containsAddress = json.contains(ADDRESS_KEY);
//    if (!containsAddress)
//    {
//        ui->proofEdit->setTextColor(Qt::red);
//        ui->proofEdit->setText(tr("The proof does not contain any address."));
//        return QString{};
//    }
//    return json[ADDRESS_KEY].toString();
//}

void CreateProofDialog::addProofs(const QStringList& proofs)
{
    ui->proofEdit->clear();
//    ui->addressesBox->clear();
    proofs_.clear();
//    showElements(true);

    if (proofs.isEmpty())
    {
        ui->proofEdit->setTextColor(Qt::red);
        ui->proofEdit->setText(tr("No proofs were generated."));
        return;
    }

    if (proofs.size() > 1)
    {
        ui->proofEdit->setTextColor(Qt::red);
        ui->proofEdit->setText(tr("Several proofs were generated."));
        return;
    }

//    for (int i = 0; i < proofs.size(); ++i)
//    {
//        const QString address = extractAddress(proofs[i]);
//        if (address.isEmpty())
//            continue;
//        proofs_.insert(i, proofs[i]);
//        ui->addressesBox->addItem(address);
//    }
    const int index = ui->addressesBox->currentIndex();
    proofs_.insert(index, proofs.front());
    ui->proofEdit->setText(proofs.front());
}

void CreateProofDialog::generateProof()
{
    emit generateProofSignal(txHash_, ui->addressesBox->currentText(), ui->messageEdit->text());
}

void CreateProofDialog::copyToClipboard()
{
    QApplication::clipboard()->setText(ui->proofEdit->toPlainText());
}

void CreateProofDialog::addressChanged(int index)
{
    ui->proofEdit->setText(proofs_[index]);
}

void CreateProofDialog::currentAddressChanged()
{
    ui->proofEdit->setText(proofs_[ui->addressesBox->currentIndex()]);
}

}
