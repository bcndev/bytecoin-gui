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

#include <QDateTime>

#include "aboutdialog.h"
#include "settings.h"
#include "ui_aboutdialog.h"

namespace WalletGUI {

AboutDialog::AboutDialog(QWidget* parent)
    : QDialog(parent, static_cast<Qt::WindowFlags>(Qt::WindowCloseButtonHint))
    , m_ui(new Ui::AboutDialog)
{
    m_ui->setupUi(this);

    QString aboutText = m_ui->m_aboutLabel->text();
    m_ui->m_aboutLabel->setText(
            aboutText
                .arg(Settings::getVersion())
                .arg(Settings::getVersionSuffix())
                .arg(Settings::getRevision())
                .arg(QDateTime::currentDateTime().date().year()));
}

AboutDialog::~AboutDialog()
{}

}
