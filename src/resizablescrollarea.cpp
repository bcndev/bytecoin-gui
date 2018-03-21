// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include "resizablescrollarea.h"

namespace WalletGUI {

ResizableScrollArea::ResizableScrollArea(QWidget* _parent) : QScrollArea(_parent) {
}

ResizableScrollArea::~ResizableScrollArea() {
}

QSize ResizableScrollArea::sizeHint() const {
  if (widget() == nullptr || widget()->height() == 0) {
    return QScrollArea::sizeHint();
  }

  QList<QWidget*> childList = widget()->findChildren<QWidget*>(QString(), Qt::FindDirectChildrenOnly);
  int height = 0;
  for (QWidget* widgetItem : childList) {
    height += widgetItem->height();
  }

  return QSize(width(), height + 2);
}

}
