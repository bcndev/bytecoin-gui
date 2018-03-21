// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

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
