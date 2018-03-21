// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#ifndef ASKPASSWORDDIALOG_H
#define ASKPASSWORDDIALOG_H

#include <QDialog>

namespace Ui {
class AskPasswordDialog;
}

namespace WalletGUI
{

class AskPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AskPasswordDialog(bool askUserName, QWidget *parent = 0);
    ~AskPasswordDialog();

    QString getPassword() const;
    QString getUser() const;

private slots:
    void passwordChanged(const QString& password);
    void userChanged(const QString& password);

private:
    Ui::AskPasswordDialog *ui;

    QString password_;
    QString user_;
};

}

#endif // ASKPASSWORDDIALOG_H
