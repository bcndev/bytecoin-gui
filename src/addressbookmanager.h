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

#include <QObject>
#include <QHash>
#include <QSettings>

namespace WalletGUI
{

using AddressIndex = int;
const int INVALID_ADDRESS_INDEX = std::numeric_limits<decltype(INVALID_ADDRESS_INDEX)>::max();

struct AddressItem
{
    QString label;
    QString address;
};

class AddressBookManager : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(AddressBookManager)

public:
  AddressBookManager(QObject* _parent);
  virtual ~AddressBookManager();

  AddressIndex getAddressCount() const;
  AddressItem getAddress(AddressIndex _addressIndex) const;
  AddressIndex findAddressByAddress(const QString& _address) const;
  AddressIndex findAddressByLabel(const QString& _label) const;
  AddressIndex findAddress(const QString& _label, const QString& _address) const;
  void addAddress(const QString& _label, const QString& _address);
  void editAddress(AddressIndex _addressIndex, const QString& _label, const QString& _address);
  void removeAddress(AddressIndex _addressIndex);

private:
  QScopedPointer<QSettings> addressBook_;
  QHash<QString, AddressIndex> addressIndexes_;
  QHash<QString, AddressIndex> labelIndexes_;

  void buildIndexes();

signals:
  void addressBookOpenedSignal();
  void addressBookClosedSignal();
  void addressAddedSignal(AddressIndex _addressIndex);
  void addressEditedSignal(AddressIndex _addressIndex);
  void addressRemovedSignal(AddressIndex _addressIndex);
};

}
