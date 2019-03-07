// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#ifndef CREATEPROOFDIALOG_H
#define CREATEPROOFDIALOG_H

#include <QDialog>
#include <QMap>

namespace Ui {
class CreateProofDialog;
}

namespace WalletGUI
{

class CreateProofDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateProofDialog(const QString& txHash, const QStringList& addresses, QWidget *parent = 0);
    ~CreateProofDialog();

public slots:
    void addProofs(const QStringList& proofs);
    void copyToClipboard();
    void addressChanged(int index);
    void currentAddressChanged();

signals:
    void generateProofSignal(const QString& txHash, const QString& address, const QString& message);

private slots:
    void generateProof();

private:
    Ui::CreateProofDialog *ui;
    QString txHash_;
    QMap<int, QString> proofs_;

//    QString extractAddress(const QString& proof);
//    void showElements(bool show);
};

}

#endif // CREATEPROOFDIALOG_H
