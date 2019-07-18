// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#include "addressbookmanager.h"
#include "logger.h"
#include "settings.h"

namespace WalletGUI
{

namespace {

constexpr const char ADDRESS_BOOK_TAG_NAME[] = "addressBook";
constexpr const char ADDRESS_ITEM_LABEL_TAG_NAME[] = "label";
constexpr const char ADDRESS_ITEM_ADDRESS_TAG_NAME[] = "address";

}

AddressBookManager::AddressBookManager(QObject* _parent)
    : IAddressBookManager(_parent)
{
    const QString jsonFile = Settings::instance().getDefaultWorkDir().absoluteFilePath("address_book.json");

    addressBook_.reset(new QSettings(jsonFile/*, jsonFormat*/, Settings::instance().getFormat()));

  buildIndexes();
  emit addressBookOpenedSignal();
}

AddressBookManager::~AddressBookManager()
{}

AddressIndex AddressBookManager::getAddressCount() const
{
    return addressBook_->value(ADDRESS_BOOK_TAG_NAME).toList().size();
}

AddressItem AddressBookManager::getAddress(AddressIndex _addressIndex) const
{
  Q_ASSERT(_addressIndex < getAddressCount());
  const QVariantMap addressObject = addressBook_->value(ADDRESS_BOOK_TAG_NAME).toList()[_addressIndex].toMap();
  AddressItem result{addressObject[ADDRESS_ITEM_LABEL_TAG_NAME].toString(), addressObject[ADDRESS_ITEM_ADDRESS_TAG_NAME].toString()};

  return result;
}

AddressIndex AddressBookManager::findAddressByAddress(const QString& _address) const
{
  return addressIndexes_.value(_address, INVALID_ADDRESS_INDEX);
}

AddressIndex AddressBookManager::findAddressByLabel(const QString& _label) const
{
  return labelIndexes_.value(_label, INVALID_ADDRESS_INDEX);
}

AddressIndex AddressBookManager::findAddress(const QString& _label, const QString& _address) const
{
    const QVariantList addressArray = addressBook_->value(ADDRESS_BOOK_TAG_NAME).toList();
    for (int i = 0; i < addressArray.size(); ++i)
    {
        const QVariantMap addressObject = addressArray[i].toMap();
        if (addressObject[ADDRESS_ITEM_LABEL_TAG_NAME].toString() == _label && addressObject[ADDRESS_ITEM_ADDRESS_TAG_NAME].toString() == _address)
            return i;
    }

    return INVALID_ADDRESS_INDEX;
}

void AddressBookManager::addAddress(const QString& _label, const QString& _address)
{
  WalletLogger::debug(tr("[AddressBook] Add address: label=\"%1\" address=\"%2\"").arg(_label).arg(_address));
  if (findAddressByLabel(_label.trimmed()) != INVALID_ADDRESS_INDEX)
  {
    WalletLogger::critical(tr("[AddressBook] Add address error. Label already exists: label=\"%1\"").arg(_label));
    return;
  }

  if (findAddressByAddress(_address.trimmed()) != INVALID_ADDRESS_INDEX)
  {
    WalletLogger::critical(tr("[AddressBook] Add address error. Address already exists: address=\"%2\"").arg(_address));
    return;
  }

  QVariantList addressArray = addressBook_->value(ADDRESS_BOOK_TAG_NAME).toList();
  QVariantMap newAddressObject;
  newAddressObject[ADDRESS_ITEM_LABEL_TAG_NAME] = _label;
  newAddressObject[ADDRESS_ITEM_ADDRESS_TAG_NAME] = _address;
  addressArray.append(newAddressObject);
  addressBook_->setValue(ADDRESS_BOOK_TAG_NAME, addressArray);
  addressIndexes_.insert(_address, addressArray.size() - 1);
  labelIndexes_.insert(_label, addressArray.size() - 1);
  emit addressAddedSignal(addressArray.size() - 1);
}

void AddressBookManager::editAddress(AddressIndex _addressIndex, const QString& _label, const QString& _address)
{
  WalletLogger::debug(tr("[AddressBook] Edit address: label=\"%1\" address=\"%2\"").arg(_label).arg(_address));
  Q_ASSERT(_addressIndex < getAddressCount());

  QVariantList addressArray = addressBook_->value(ADDRESS_BOOK_TAG_NAME).toList();
  QVariantMap addressObject = addressArray[_addressIndex].toMap();

  const QString oldAddress = addressObject[ADDRESS_ITEM_ADDRESS_TAG_NAME].toString();
  const QString oldLabel = addressObject[ADDRESS_ITEM_LABEL_TAG_NAME].toString();
  addressObject[ADDRESS_ITEM_LABEL_TAG_NAME] = _label;
  addressObject[ADDRESS_ITEM_ADDRESS_TAG_NAME] = _address;
  addressArray[_addressIndex] = addressObject;
  addressBook_->setValue(ADDRESS_BOOK_TAG_NAME, addressArray);
  addressIndexes_.remove(oldAddress);
  labelIndexes_.remove(oldLabel);
  addressIndexes_.insert(_address, _addressIndex);
  labelIndexes_.insert(_label, _addressIndex);
  emit addressEditedSignal(_addressIndex);
}

void AddressBookManager::removeAddress(AddressIndex _addressIndex)
{
  Q_ASSERT(_addressIndex < getAddressCount());
    QVariantList addressArray = addressBook_->value(ADDRESS_BOOK_TAG_NAME).toList();
    QVariantMap addressObject = addressArray[_addressIndex].toMap();
    const QString oldAddress = addressObject[ADDRESS_ITEM_ADDRESS_TAG_NAME].toString();
    const QString oldLabel = addressObject[ADDRESS_ITEM_LABEL_TAG_NAME].toString();
  WalletLogger::debug(tr("[AddressBook] Remove address: label=\"%1\" address=\"%2\"").arg(oldLabel).arg(oldAddress));

  emit beginRemoveAddressSignal(_addressIndex);

  addressArray.removeAt(_addressIndex);
  addressIndexes_.remove(oldAddress);
  labelIndexes_.remove(oldLabel);
  for (AddressIndex i = _addressIndex; i < addressArray.size(); ++i)
  {
    const QVariantMap addressObject = addressArray[i].toMap();
    const QString address = addressObject.value(ADDRESS_ITEM_ADDRESS_TAG_NAME).toString();
    const QString label = addressObject.value(ADDRESS_ITEM_LABEL_TAG_NAME).toString();
    addressIndexes_[address] = i;
    labelIndexes_[label] = i;
  }

  addressBook_->setValue(ADDRESS_BOOK_TAG_NAME, addressArray);
//  emit addressRemovedSignal(_addressIndex);
  emit endRemoveAddressSignal();
}

void AddressBookManager::buildIndexes() {
  const QVariantList addressArray = addressBook_->value(ADDRESS_BOOK_TAG_NAME).toList();
  for (int i = 0; i < addressArray.size(); ++i) {
      const QVariantMap addressObject = addressArray[i].toMap();
      const QString address = addressObject.value(ADDRESS_ITEM_ADDRESS_TAG_NAME).toString();
      const QString label = addressObject.value(ADDRESS_ITEM_LABEL_TAG_NAME).toString();
      addressIndexes_[address] = i;
      labelIndexes_[label] = i;
  }
}

MyAddressesManager::MyAddressesManager(QObject* parent)
    : IAddressBookManager(parent)
    , firstTime_(true)
{}

MyAddressesManager::~MyAddressesManager()
{}

void MyAddressesManager::connectedToWalletd()
{
    WalletLogger::debug(tr("[WalletAddressBook] Connected to walletd."));
    firstTime_ = true;
    addressBook_.clear();
    requestAddresses();
}

void MyAddressesManager::disconnectedFromWalletd()
{
    emit addressBookClosedSignal();
}

void MyAddressesManager::requestAddresses(quint32 index, quint32 count)
{
    emit getWalletRecordsSignal(RpcApi::GetWalletRecords::Request{false, false, index, count});
}

void MyAddressesManager::createAddress(const QString& label)
{
    emit createAddressSignal(label);
}

void MyAddressesManager::setAddressLabel(const QString& address, const QString& label)
{
    emit setAddressLabelSignal(RpcApi::SetAddressLabel::Request{address, label});
}

void MyAddressesManager::walletRecordsReceived(const RpcApi::WalletRecords& records)
{
    WalletLogger::debug(tr("[WalletAddressBook] Wallet records received."));
    for (const RpcApi::WalletRecord& rec: records.records)
    {
        addressBook_ << AddressItem{rec.label, rec.address};
        const AddressIndex index = addressBook_.size() - 1;
        addressIndexes_[rec.address] = index;
//        WalletLogger::debug(tr("[WalletAddressBook] Wallet record indexed."));
        if (!firstTime_)
            emit addressAddedSignal(index);
    }

    if (firstTime_)
    {
        firstTime_ = false;
        emit addressBookOpenedSignal();
    }
}

void MyAddressesManager::addressLabelSetReceived(const QString& address, const QString& label)
{
    const AddressIndex index = findAddressByAddress(address);
    if (index == INVALID_ADDRESS_INDEX)
    {
        WalletLogger::warning(tr("[WalletAddressBook] Failed to find address %1.").arg(address));
        return;
    }
    addressBook_[index].label = label;
    WalletLogger::debug(tr("[WalletAddressBook] Label changed."));
    emit addressEditedSignal(index);
}

AddressIndex MyAddressesManager::getAddressCount() const
{
    return addressBook_.size();
}

AddressItem MyAddressesManager::getAddress(AddressIndex addressIndex) const
{
    Q_ASSERT(addressIndex < getAddressCount());
    return addressBook_[addressIndex];
}

void MyAddressesManager::addAddress(const QString& label, const QString& address)
{
    Q_ASSERT(address.isEmpty());
    createAddress(label);
}

void MyAddressesManager::addressCreatedReceived(const RpcApi::CreatedAddresses& addrs)
{
    Q_ASSERT(addrs.addresses.size() == 1);
    WalletLogger::debug(tr("[WalletAddressBook] New address created."));
    requestAddresses(addressBook_.size(), 1);
}

void MyAddressesManager::editAddress(AddressIndex addressIndex, const QString& label, const QString& address)
{
    Q_ASSERT(addressIndex < getAddressCount());
    Q_ASSERT(address.isEmpty());
    setAddressLabel(addressBook_[addressIndex].address, label);
}

void MyAddressesManager::removeAddress(AddressIndex /*addressIndex*/)
{
    Q_ASSERT(false); // addresses cannot be removed from wallet
}

//void MyAddressesManager::buildIndexes()
//{
//    for (int i = 0, size = addressBook_.size(); i < size; ++i)
//    {
//        const AddressItem& item = addressBook_[i];
//        addressIndexes_[item.address] = i;
//    }
//}

AddressIndex MyAddressesManager::findAddressByAddress(const QString& address) const
{
  return addressIndexes_.value(address, INVALID_ADDRESS_INDEX);
}


}
