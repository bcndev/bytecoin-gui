// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QSortFilterProxyModel>

namespace WalletGUI
{

class SortedAddressBookModel : public QSortFilterProxyModel
{
  Q_OBJECT
  Q_DISABLE_COPY(SortedAddressBookModel)

public:
  SortedAddressBookModel(QAbstractItemModel* _sourceModel, QObject* _parent);
  ~SortedAddressBookModel();

private:
  virtual bool lessThan(const QModelIndex& _left, const QModelIndex& _right) const override;
};

}
