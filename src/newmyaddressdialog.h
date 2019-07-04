// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QDialog>

class QPersistentModelIndex;

namespace Ui {
class NewMyAddressDialog;
}

namespace WalletGUI {

class IAddressBookManager;

class NewMyAddressDialog : public QDialog
{
  Q_OBJECT
  Q_DISABLE_COPY(NewMyAddressDialog)

public:
  NewMyAddressDialog(IAddressBookManager* addressBookManager, QWidget* _parent);
  NewMyAddressDialog(IAddressBookManager* addressBookManager, const QPersistentModelIndex& _index, QWidget* _parent);
  ~NewMyAddressDialog();

  QString getAddress() const;
  QString getLabel() const;
//  bool hasDonationFlag() const;

//  void setAddressError(bool _error);
//  void setAddressDuplicationError(bool _error);
//  void setLabelDuplicationError(bool _error);

private:
  QScopedPointer<Ui::NewMyAddressDialog> m_ui;
  IAddressBookManager* m_addressBookManager;

//  bool checkForErrors() const;

//  Q_SLOT void validateAddress(const QString& _address);
//  Q_SLOT void validateLabel(const QString& _label);
};

}
