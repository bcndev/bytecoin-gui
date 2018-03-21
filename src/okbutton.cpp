// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include "okbutton.h"

namespace WalletGUI {

namespace {

//const char WALLET_OK_BUTTON_STYLE_SHEET_TEMPLATE[] =
//  "* {"
//    "font-family: %fontFamily%;"
//  "}"

//  "WalletGui--WalletOkButton {"
//    "min-width: 80px;"
//    "max-width: 80px;"
//    "min-height: 30px;"
//    "max-height: 30px;"
//    "font-size: %fontSizeNormal%;"
//    "color: #ffffff;"
//    "background-color: %backgroundColorBlueNormal%;"
//    "border: 1px solid %backgroundColorBlueNormal%;"
//  "}"

//  "WalletGui--WalletOkButton:disabled {"
//    "background-color: %backgroundColorBlueDisabled%;"
//    "border: 1px solid %backgroundColorBlueDisabled%;"
//  "}"

//  "WalletGui--WalletOkButton:hover {"
//    "background-color: %backgroundColorBlueHover%;"
//    "border: 1px solid %backgroundColorBlueHover%;"
//  "}"

//  "WalletGui--WalletOkButton:pressed {"
//    "background-color: %backgroundColorBluePressed%;"
//    "border: 1px solid %backgroundColorBluePressed%;"
//  "}";

}

WalletOkButton::WalletOkButton(QWidget* parent)
    : QPushButton(parent)
{
//    setStyleSheet(Settings::instance().getCurrentStyle().makeStyleSheet(WALLET_OK_BUTTON_STYLE_SHEET_TEMPLATE));
}

WalletOkButton::~WalletOkButton()
{}

}
