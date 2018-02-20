#include "askpassworddialog.h"
#include "ui_askpassworddialog.h"

namespace WalletGUI
{

AskPasswordDialog::AskPasswordDialog(bool askUserName, QWidget *parent)
    : QDialog(parent, Qt::Dialog)
    , ui(new Ui::AskPasswordDialog)
{
    ui->setupUi(this);

    ui->userLabel->setVisible(askUserName);
    ui->userEdit->setVisible(askUserName);
    adjustSize();
}

AskPasswordDialog::~AskPasswordDialog()
{
    password_.fill('0', 200);
    password_.clear();

    delete ui;
}

void AskPasswordDialog::passwordChanged(const QString& password)
{
    password_ = password;
}

void AskPasswordDialog::userChanged(const QString& user)
{
    user_ = user;
}

QString AskPasswordDialog::getPassword() const
{
    return password_;
}

QString AskPasswordDialog::getUser() const
{
    return user_;
}

}
