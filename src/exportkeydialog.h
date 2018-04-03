#ifndef EXPORTKEYDIALOG_H
#define EXPORTKEYDIALOG_H

#include <QDialog>

namespace Ui {
class ExportKeyDialog;
}

namespace WalletGUI
{

class ExportKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportKeyDialog(const QString& key, QWidget *parent = 0);
    ~ExportKeyDialog();

public slots:
    void saveKey();

private:
    Ui::ExportKeyDialog *ui;
};

}

#endif // EXPORTKEYDIALOG_H
