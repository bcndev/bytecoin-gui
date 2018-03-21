// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#ifndef WALLETGUI_CHANGEPASSWORDDIALOG_H
#define WALLETGUI_CHANGEPASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
class ChangePasswordDialog;
}

namespace WalletGUI {

class ChangePasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChangePasswordDialog(bool askCurrentPassword, QWidget *parent = 0);
    ~ChangePasswordDialog();

    QString getOldPassword() const;
    QString getNewPassword() const;

private slots:
    void passwordChanged(const QString& password);
    void confirmationChanged(const QString& confirmation);

private:
    Ui::ChangePasswordDialog *ui;
    QString oldPassword_;
    QString newPassword_;      // TODO: secure functions
    QString confirmation_;

    void updateOkButton();
};

} // namespace WalletGUI
#endif // WALLETGUI_CHANGEPASSWORDDIALOG_H
