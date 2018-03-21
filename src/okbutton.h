// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QPushButton>

namespace WalletGUI {

class WalletOkButton : public QPushButton
{
    Q_OBJECT
    Q_DISABLE_COPY(WalletOkButton)

public:
    explicit WalletOkButton(QWidget* parent);
    ~WalletOkButton();
};

}
