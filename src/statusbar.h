// Copyright (c) 2015-2017, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <QStatusBar>

class QLabel;
class QAbstractItemModel;
class QDataWidgetMapper;

namespace WalletGUI {

class WalletModel;

class WalletStatusBar : public QStatusBar
{
    Q_OBJECT
    Q_DISABLE_COPY(WalletStatusBar)

public:
    explicit WalletStatusBar(QWidget* parent);
    virtual ~WalletStatusBar();

    void setWalletModel(WalletModel* model);

private:
    WalletModel* walletModel_;
    QLabel* m_syncStatusLabel;
    QLabel* m_syncStatusIconLabel;
    QLabel* m_encryptionStatusIconLabel;
    QLabel* m_peerCountLabel;
    QLabel* m_walletConnectionLabel;
    QMovie* m_syncMovie;
    QDataWidgetMapper* stateMapper_;
    bool isSynchronized_;

    void nodeStateChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void updateStatusDescription();
    void updateSyncState();
    void updateEncryptedState(bool isEncrypted);
};

}
