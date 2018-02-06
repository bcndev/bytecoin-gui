#ifndef CONNECTSELECTIONDIALOG_H
#define CONNECTSELECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ConnectSelectionDialog;
}

namespace WalletGUI
{

class ConnectSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectSelectionDialog(QWidget *parent = 0);
    ~ConnectSelectionDialog();

    Q_SLOT void accept() override;

private:
    Ui::ConnectSelectionDialog *ui;
};

}

#endif // CONNECTSELECTIONDIALOG_H
