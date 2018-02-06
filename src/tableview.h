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

#include <QTableView>

namespace WalletGUI {

class WalletTableView : public QTableView
{
    Q_OBJECT
    Q_DISABLE_COPY(WalletTableView)

public:
    explicit WalletTableView(QWidget* parent);
    virtual ~WalletTableView();

    void setLinkLikeColumnSet(const QSet<int>& linkLikeColumnSet);
    void setCopyableColumnSet(const QSet<int>& copyableColumnSet);
    void setHoverIsVisible(bool hoverIsVisible);
    void updateRowCount();
    void setAutoUpdateRowCount(bool autoUpdateRowCount);

    virtual void setModel(QAbstractItemModel* model) override;

protected:
    virtual void mouseMoveEvent(QMouseEvent* event) override;
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    QSet<int> m_linkLikeColumnSet;
    QSet<int> m_copyableColumnSet;
//    LinkLikeColumnDelegate* m_linkLikeColumnDelegate;
    bool m_autoUpdateRowCount;

    void itemClicked(const QModelIndex& index);
    int calculateVisibleRowCount() const;

Q_SIGNALS:
    void copyableItemClickedSignal(const QModelIndex& index);
};

}
