// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QFile>
#include <QFont>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaEnum>
#include <QPixmap>
#include <QSize>

#include "addressbookmodel.h"
#include "addressbookmanager.h"
#include "settings.h"

namespace WalletGUI
{

AddressBookModel::AddressBookModel(IAddressBookManager* _addressBookManager, QObject* _parent)
    : QAbstractItemModel(_parent)
    , m_addressBookManager(_addressBookManager)
    , m_columnCount(AddressBookModel::staticMetaObject.enumerator(AddressBookModel::staticMetaObject.indexOfEnumerator("Columns")).keyCount())
    , m_rowCount(0)
{
    connect(m_addressBookManager, &IAddressBookManager::addressBookOpenedSignal, this, &AddressBookModel::addressBookOpened);
    connect(m_addressBookManager, &IAddressBookManager::addressBookClosedSignal, this, &AddressBookModel::addressBookClosed);
    connect(m_addressBookManager, &IAddressBookManager::addressAddedSignal, this, &AddressBookModel::addressAdded);
    connect(m_addressBookManager, &IAddressBookManager::addressEditedSignal, this, &AddressBookModel::addressEdited);
//    connect(m_addressBookManager, &IAddressBookManager::addressRemovedSignal, this, &AddressBookModel::addressRemoved);
    connect(m_addressBookManager, &IAddressBookManager::beginRemoveAddressSignal, this, &AddressBookModel::beginRemoveAddress);
    connect(m_addressBookManager, &IAddressBookManager::endRemoveAddressSignal, this, &AddressBookModel::endRemoveAddress);

    addressBookOpened();
}

AddressBookModel::~AddressBookModel()
{
    addressBookClosed();
}

int AddressBookModel::columnCount(const QModelIndex& /*_parent*/) const {
  return m_columnCount;
}

QVariant AddressBookModel::data(const QModelIndex& _index, int _role) const {
  if (!_index.isValid()) {
    return QVariant();
  }

  switch(_role) {
  case Qt::DisplayRole:
    return getDisplayRole(_index);
  case Qt::DecorationRole:
    return getDecorationRole(_index);
  case Qt::TextAlignmentRole:
    return headerData(_index.column(), Qt::Horizontal, _role);
  default:
    return getUserRole(_index, _role);
    break;
  }

  return QVariant();
}

Qt::ItemFlags AddressBookModel::flags(const QModelIndex& /*_index*/) const {
  return (Qt::ItemIsEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsSelectable);
}

QVariant AddressBookModel::headerData(int _section, Qt::Orientation _orientation, int _role) const {
  if (_orientation != Qt::Horizontal) {
    return QVariant();
  }

  switch(_role) {
  case Qt::DisplayRole: {
    switch(_section) {
    case COLUMN_LABEL:
      return tr("Label");
    case COLUMN_ADDRESS:
      return tr("Address");
    }

    break;
  }

  case Qt::TextAlignmentRole: {
    switch(_section) {
    case COLUMN_LABEL:
      return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    case COLUMN_ADDRESS:
      return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    }

    break;
  }

  case ROLE_COLUMN:
    return _section;
  default:
    break;
  }

  return QVariant();
}

QModelIndex AddressBookModel::index(int _row, int _column, const QModelIndex& _parent) const {
  if (_parent.isValid()) {
    return QModelIndex();
  }

  return createIndex(_row, _column, _row);
}

QModelIndex AddressBookModel::parent(const QModelIndex& /*_index*/) const {
  return QModelIndex();
}

int AddressBookModel::rowCount(const QModelIndex& /*_parent*/) const {
  return m_rowCount;
}

void AddressBookModel::addressBookOpened() {
  if (m_addressBookManager->getAddressCount() > 0) {
    beginInsertRows(QModelIndex(), 0, m_addressBookManager->getAddressCount() - 1);
    m_rowCount = m_addressBookManager->getAddressCount();
    endInsertRows();
  }
}

void AddressBookModel::addressBookClosed() {
  if (m_rowCount > 0) {
    beginRemoveRows(QModelIndex(), 0, m_rowCount - 1);
    m_rowCount = 0;
    endRemoveRows();
  }
}

void AddressBookModel::addressAdded(quintptr /*_addressIndex*/) {
  quintptr newRowCount = m_addressBookManager->getAddressCount();
  if (m_rowCount < newRowCount) {
    beginInsertRows(QModelIndex(), m_rowCount, newRowCount - 1);
    m_rowCount = newRowCount;
    endInsertRows();
  }
}

void AddressBookModel::addressEdited(quintptr _addressIndex) {
  Q_EMIT dataChanged(index(_addressIndex, 0), index(_addressIndex, columnCount() - 1));
}

//void AddressBookModel::addressRemoved(quintptr _addressIndex) {
//  beginRemoveRows(QModelIndex(), _addressIndex, _addressIndex);
//  m_rowCount = m_addressBookManager->getAddressCount();
//  endRemoveRows();
//}

void AddressBookModel::beginRemoveAddress(quintptr _addressIndex)
{
    beginRemoveRows(QModelIndex(), _addressIndex, _addressIndex);
}

void AddressBookModel::endRemoveAddress()
{
    m_rowCount = m_addressBookManager->getAddressCount();
    endRemoveRows();
}

QVariant AddressBookModel::getDisplayRole(const QModelIndex& _index) const {
  switch(_index.column()) {
  case COLUMN_LABEL:
    return _index.data(ROLE_LABEL);
  case COLUMN_ADDRESS:
    return _index.data(ROLE_ADDRESS);
  default:
    break;
  }

  return QVariant();
}

QVariant AddressBookModel::getDecorationRole(const QModelIndex& /*_index*/) const {
  return QVariant();
}

QVariant AddressBookModel::getUserRole(const QModelIndex& _index, int _role) const {
  AddressItem addressItem = m_addressBookManager->getAddress(_index.row());
  switch (_role) {
  case ROLE_LABEL:
    return addressItem.label;
  case ROLE_ADDRESS:
    return addressItem.address;
  case ROLE_COLUMN:
    return headerData(_index.column(), Qt::Horizontal, _role);
  case ROLE_ROW:
    return _index.row();
  }

  return QVariant();
}

}
