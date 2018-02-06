#ifndef WALLETGUI_CRASHDIALOG_H
#define WALLETGUI_CRASHDIALOG_H

#include <QDialog>

namespace Ui {
class CrashDialog;
}

namespace WalletGUI {

class MainWindow;

class CrashDialog : private QDialog
{
    Q_OBJECT

public:
    explicit CrashDialog(QWidget *parent = 0);
    ~CrashDialog();

    void setMainWindow(MainWindow* mainWindow);
    int execWithReason(const QString& reason, bool enablePassword);
    QString getPassword() const;

    using QDialog::accept;

public slots:
    void showDaemonConsole();

private:
    void enablePasswordEdit(bool enable);

    Ui::CrashDialog *ui;
    MainWindow* mainWindow_;
};


} // namespace WalletGUI
#endif // WALLETGUI_CRASHDIALOG_H
