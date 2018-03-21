// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QScrollArea>

namespace WalletGUI {

class ResizableScrollArea : public QScrollArea {
  Q_OBJECT
  Q_DISABLE_COPY(ResizableScrollArea)

public:
  explicit ResizableScrollArea(QWidget* _parent);
  virtual ~ResizableScrollArea();

  virtual QSize sizeHint() const override;
};

}
