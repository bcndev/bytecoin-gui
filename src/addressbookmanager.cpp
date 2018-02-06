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
    : QObject(_parent)
{
//    const QSettings::Format jsonFormat = QSettings::registerFormat("json", readJsonFile, writeJsonFile);
    const QString jsonFile = Settings::instance().getDefaultWorkDir().absoluteFilePath("address_book.json");

    addressBook_.reset(new QSettings(jsonFile/*, jsonFormat*/, Settings::instance().getFormat()));

  buildIndexes();
  emit addressBookOpenedSignal();
}

AddressBookManager::~AddressBookManager()
{}

AddressIndex AddressBookManager::getAddressCount() const
{
//  return m_addressBookObject[ADDRESS_BOOK_TAG_NAME].toArray().size();
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
//  saveAddressBook();
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
//  saveAddressBook();
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
//  saveAddressBook();
  emit addressRemovedSignal(_addressIndex);
}

//void AddressBookManager::addObserver(IAddressBookManagerObserver* _observer) {
//  QObject* observer = dynamic_cast<QObject*>(_observer);
//  connect(this, SIGNAL(addressBookOpenedSignal()), observer, SLOT(addressBookOpened()));
//  connect(this, SIGNAL(addressBookClosedSignal()), observer, SLOT(addressBookClosed()));
//  connect(this, SIGNAL(addressAddedSignal(AddressIndex)), observer, SLOT(addressAdded(AddressIndex)));
//  connect(this, SIGNAL(addressEditedSignal(AddressIndex)), observer, SLOT(addressEdited(AddressIndex)));
//  connect(this, SIGNAL(addressRemovedSignal(AddressIndex)), observer, SLOT(addressRemoved(AddressIndex)));
//}

//void AddressBookManager::removeObserver(IAddressBookManagerObserver* _observer) {
//  QObject* observer = dynamic_cast<QObject*>(_observer);
//  disconnect(this, SIGNAL(addressBookOpenedSignal()), observer, SLOT(addressBookOpened()));
//  disconnect(this, SIGNAL(addressBookClosedSignal()), observer, SLOT(addressBookClosed()));
//  disconnect(this, SIGNAL(addressAddedSignal(AddressIndex)), observer, SLOT(addressAdded(AddressIndex)));
//  disconnect(this, SIGNAL(addressEditedSignal(AddressIndex)), observer, SLOT(addressEdited(AddressIndex)));
//  disconnect(this, SIGNAL(addressRemovedSignal(AddressIndex)), observer, SLOT(addressRemoved(AddressIndex)));
//}

//void AddressBookManager::walletOpened() {
//  QByteArray userData = m_cryptoNoteAdapter->getNodeAdapter()->getWalletAdapter()->getUserData();
//  bool needOverwrite = false;
//  if (userData.isEmpty()) {
//    QFile addressBookFile(Settings::instance().getLegacyAddressBookFile());
//    if (addressBookFile.open(QIODevice::ReadOnly)) {
//      needOverwrite = true;
//      userData = addressBookFile.readAll();
//    }
//  }

//  if (!userData.isEmpty()) {
//    QJsonDocument doc = QJsonDocument::fromBinaryData(qUncompress(userData));
//    if (doc.isNull()) {
//      doc = QJsonDocument::fromJson(userData);
//      if (!doc.isNull()) {
//        needOverwrite = true;
//      }
//    }

//    if (!doc.isNull()) {
//      if (doc.isArray()) {
//        m_addressBookObject.insert(ADDRESS_BOOK_TAG_NAME, doc.array());
//        needOverwrite = true;
//      } else  {
//        m_addressBookObject = doc.object();
//      }

//      if (needOverwrite) {
//        saveAddressBook();
//      }
//    }
//  }

//  buildIndexes();
//  Q_EMIT addressBookOpenedSignal();
//  Q_EMIT donationManagerOpenedSignal();
//}

void AddressBookManager::saveAddressBook()
{
//    m_cryptoNoteAdapter->getNodeAdapter()->getWalletAdapter()->setUserData(qCompress(QJsonDocument(m_addressBookObject).toBinaryData()));

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

}
