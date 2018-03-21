// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

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
