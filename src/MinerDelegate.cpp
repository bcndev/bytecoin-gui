// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QAbstractItemView>

#include "MinerDelegate.h"
#include "MinerModel.h"

namespace WalletGUI {

MinerRemoveDelegate::MinerRemoveDelegate(QAbstractItemView* _view, QObject* _parent) : QStyledItemDelegate(_parent),
  m_view(_view) {
}

MinerRemoveDelegate::~MinerRemoveDelegate() {
}

void MinerRemoveDelegate::paint(QPainter* _painter, const QStyleOptionViewItem& _option, const QModelIndex& _index) const {
  if (_index.column() == MinerModel::COLUMN_REMOVE) {
    QStyleOptionViewItem opt(_option);
    if ((opt.state & QStyle::State_MouseOver) && opt.rect.contains(m_view->viewport()->mapFromGlobal(QCursor::pos()))) {
      initStyleOption(&opt, _index);
      QRect pixmapRect = opt.widget->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt, opt.widget);
      opt.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, _painter, opt.widget);
      opt.widget->style()->drawItemPixmap(_painter, pixmapRect, opt.displayAlignment, QPixmap(":icons/delete_pool_hover"));
      return;
    }
  }

  QStyledItemDelegate::paint(_painter, _option, _index);
}

}
