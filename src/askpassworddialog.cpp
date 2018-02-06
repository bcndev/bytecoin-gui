#include "askpassworddialog.h"
#include "ui_askpassworddialog.h"

namespace WalletGUI
{

AskPasswordDialog::AskPasswordDialog(QWidget *parent) :
    QDialog(parent, Qt::Dialog),
    ui(new Ui::AskPasswordDialog)
{
    ui->setupUi(this);

//    setWindowTitle("Bytecoin");
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

QString AskPasswordDialog::getPassword() const
{
    return password_;
}

}
