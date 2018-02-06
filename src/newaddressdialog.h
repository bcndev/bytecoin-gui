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

#include <QDialog>

class QPersistentModelIndex;

namespace Ui {
class NewAddressDialog;
}

namespace WalletGUI {

class AddressBookManager;

class NewAddressDialog : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(NewAddressDialog)

public:
  NewAddressDialog(AddressBookManager* addressBookManager, QWidget* _parent);
  NewAddressDialog(AddressBookManager* addressBookManager, const QPersistentModelIndex& _index, QWidget* _parent);
  ~NewAddressDialog();

  QString getAddress() const;
  QString getLabel() const;
  bool hasDonationFlag() const;

  void setAddressError(bool _error);
  void setAddressDuplicationError(bool _error);
  void setLabelDuplicationError(bool _error);

private:
  QScopedPointer<Ui::NewAddressDialog> m_ui;
  AddressBookManager* m_addressBookManager;

  bool checkForErrors() const;

  Q_SLOT void validateAddress(const QString& _address);
  Q_SLOT void validateLabel(const QString& _label);
};

}
