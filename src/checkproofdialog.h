// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#ifndef CHECKPROOFDIALOG_H
#define CHECKPROOFDIALOG_H

#include <QDialog>
#include "rpcapi.h"

namespace Ui {
class CheckProofDialog;
}

namespace WalletGUI
{

class CheckProofDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CheckProofDialog(QWidget *parent = 0);
    ~CheckProofDialog();

public slots:
    void showCheckResult(const RpcApi::ProofCheck& result);

signals:
    void checkProofSignal(const QString& proof);

private:
    Ui::CheckProofDialog *ui;

    void clear();

private slots:
    void pasteFromClipboard();
    void proofChanged();
    void checkProof();

};

}

#endif // CHECKPROOFDIALOG_H
