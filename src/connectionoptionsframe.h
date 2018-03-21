// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QFrame>

namespace Ui {
  class ConnectionOptionsFrame;
}

namespace WalletGUI
{

enum class NodeType
{
    UNKNOWN, IN_PROCESS, RPC
};

class ConnectionOptionsFrame : public QFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(ConnectionOptionsFrame)

public:
    explicit ConnectionOptionsFrame(QWidget* parent = nullptr);
    ~ConnectionOptionsFrame();

    void load();
    void save();
    bool canAccept() const;

private:
    QScopedPointer<Ui::ConnectionOptionsFrame> m_ui;

    Q_SLOT bool remoteHostNameChanged(const QString& host);
    Q_SLOT bool walletFileNameChanged(const QString& wallet);
    Q_SLOT void connectionButtonClicked(int buttonId);
    Q_SLOT void showOpenWalletFileDialog();
    Q_SLOT void showCreateWalletFileDialog();

Q_SIGNALS:
    void showRestartWarningSignal(bool show);
    void disableAcceptButtonSignal(bool disable);
};

}
