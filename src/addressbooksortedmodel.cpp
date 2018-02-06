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

#include "addressbooksortedmodel.h"
#include "addressbookmodel.h"

namespace WalletGUI
{

SortedAddressBookModel::SortedAddressBookModel(QAbstractItemModel* _sourceModel, QObject* _parent)
    : QSortFilterProxyModel(_parent)
{
  setSourceModel(_sourceModel);
  setDynamicSortFilter(true);
  sort(AddressBookModel::COLUMN_LABEL, Qt::DescendingOrder);
}

SortedAddressBookModel::~SortedAddressBookModel()
{}

bool SortedAddressBookModel::lessThan(const QModelIndex& _left, const QModelIndex& _right) const
{
  QString leftLabel = _left.data(AddressBookModel::ROLE_LABEL).toString();
  QString rightLabel = _right.data(AddressBookModel::ROLE_LABEL).toString();

  return QString::compare(leftLabel, rightLabel, Qt::CaseInsensitive) > 0;
}

}
