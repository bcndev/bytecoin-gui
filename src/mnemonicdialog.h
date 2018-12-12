// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#ifndef MNEMONICDIALOG_H
#define MNEMONICDIALOG_H

#include <QDialog>

namespace Ui {
class MnemonicDialog;
}

namespace WalletGUI
{

class MnemonicDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MnemonicDialog(bool generate, QWidget *parent = 0);
    ~MnemonicDialog();

    QByteArray getMnemonic() const;

private slots:
    void generateMnemonic();
    void mnemonicChanged();

private:
    Ui::MnemonicDialog *ui;
    QWidget* parent_;

    void daemonStandardErrorReady(QString err);
};

}

#endif // MNEMONICDIALOG_H
