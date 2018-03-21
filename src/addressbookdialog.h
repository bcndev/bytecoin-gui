// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QDialog>

class QAbstractItemModel;

namespace Ui {
class AddressBookDialog;
}

namespace WalletGUI {

class AddressBookDialog : public QDialog {
  Q_OBJECT
  Q_DISABLE_COPY(AddressBookDialog)

public:
  AddressBookDialog(QAbstractItemModel* _addressBookModel, QWidget* _parent);
  ~AddressBookDialog();

  QString getAddress() const;
  QString getLabel() const;

private:
  QScopedPointer<Ui::AddressBookDialog> m_ui;
};

}
