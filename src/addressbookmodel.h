// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QAbstractItemModel>
#include <QJsonObject>

namespace WalletGUI {

class IAddressBookManager;

class AddressBookModel : public QAbstractItemModel
{
  Q_OBJECT
  Q_DISABLE_COPY(AddressBookModel)
  Q_ENUMS(Columns)
  Q_ENUMS(Roles)

public:
  enum Columns {
    COLUMN_LABEL = 0, COLUMN_ADDRESS, /*COLUMN_DONATION,*/ /*COLUMN_ACTION*/
  };

  enum Roles {
    ROLE_LABEL = Qt::UserRole, ROLE_ADDRESS, /*ROLE_IS_DONATION_ADDRESS,*/ ROLE_COLUMN, ROLE_ROW
  };

  explicit AddressBookModel(IAddressBookManager* _addressBookManager, QObject* _parent);
  ~AddressBookModel();

  int columnCount(const QModelIndex& _parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& _index, int _role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex& _index) const override;
  QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const override;
  QModelIndex index(int _row, int _column, const QModelIndex& _parent = QModelIndex()) const override;
  QModelIndex	parent(const QModelIndex& _index) const override;
  int rowCount(const QModelIndex& _parent = QModelIndex()) const override;

  // IAddressBookManagerObserver
  Q_SLOT void addressBookOpened();
  Q_SLOT void addressBookClosed();
  Q_SLOT void addressAdded(quintptr _addressIndex);
  Q_SLOT void addressEdited(quintptr _addressIndex);
//  Q_SLOT void addressRemoved(quintptr _addressIndex);
  Q_SLOT void beginRemoveAddress(quintptr _addressIndex);
  Q_SLOT void endRemoveAddress();

private:
  IAddressBookManager* m_addressBookManager;
  const int m_columnCount;
  quintptr m_rowCount;

  QVariant getDisplayRole(const QModelIndex& _index) const;
  QVariant getDecorationRole(const QModelIndex& _index) const;
  QVariant getUserRole(const QModelIndex& _index, int _role) const;
};

}
