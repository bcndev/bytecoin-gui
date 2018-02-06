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
    explicit AskPasswordDialog(QWidget *parent = 0);
    ~AskPasswordDialog();

    QString getPassword() const;

private slots:
    void passwordChanged(const QString& password);

private:
    Ui::AskPasswordDialog *ui;

    QString password_;
};

}

#endif // ASKPASSWORDDIALOG_H
