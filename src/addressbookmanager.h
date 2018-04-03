// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

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
//  void addressRemovedSignal(AddressIndex _addressIndex);
  void beginRemoveAddressSignal(AddressIndex _addressIndex);
  void endRemoveAddressSignal();
};

}
