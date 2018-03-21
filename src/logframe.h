// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#ifndef LOGFRAME_H
#define LOGFRAME_H

#include <QFrame>
#include <QColor>

class QMutex;

namespace Ui {
class LogFrame;
}

namespace WalletGUI {

class LogFrame : public QFrame
{
    Q_OBJECT

public:
    explicit LogFrame(QWidget *parent = 0);
    ~LogFrame();

public slots:
    void addDaemonOutput(const QString& data);
    void addDaemonError(const QString& data);
    void addGuiMessage(const QString& data);
    void addNetworkMessage(const QString& data);
    void copyToClipboard();

private:
    Ui::LogFrame *ui;
    QColor defaultColor_;
    QMutex* mutex_;
    bool autoScroll_;

    void scrollDown();
    void print(const QString& data, const QColor& color = QColor());
    void insertText(const QString& data, const QColor& color);

private slots:
    void scrolled(int value);
};

}

#endif // LOGFRAME_H
