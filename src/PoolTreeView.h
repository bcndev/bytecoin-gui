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

#include <QDateTime>
#include <QTableView>

//#include "Common/WalletTreeView.h"

namespace WalletGUI
{

class PoolTreeView : public QTableView
{
    Q_OBJECT
    Q_DISABLE_COPY(PoolTreeView)

public:
  explicit PoolTreeView(QWidget* parent);
  ~PoolTreeView();

  bool eventFilter(QObject* object, QEvent* event) override;

protected:
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

private:
  QDateTime m_pressedTime;
  QPoint m_pressedPosition;
  QModelIndex m_pressedIndex;
};

}
