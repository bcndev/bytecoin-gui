// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QObject>
#include <QHash>
#include <QSettings>
#include "rpcapi.h"

namespace WalletGUI
{

using AddressIndex = int;
const int INVALID_ADDRESS_INDEX = std::numeric_limits<decltype(INVALID_ADDRESS_INDEX)>::max();

struct AddressItem
{
    QString label;
    QString address;
};

class IAddressBookManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IAddressBookManager)

public:
    IAddressBookManager(QObject* _parent)
        : QObject(_parent) {}
    virtual ~IAddressBookManager() {}

    virtual AddressIndex getAddressCount() const = 0;
    virtual AddressItem getAddress(AddressIndex _addressIndex) const = 0;
//    virtual AddressIndex findAddressByAddress(const QString& _address) const = 0;
//    virtual AddressIndex findAddressByLabel(const QString& _label) const = 0;
//    virtual AddressIndex findAddress(const QString& _label, const QString& _address) const = 0;
    virtual void addAddress(const QString& _label, const QString& _address) = 0;
    virtual void editAddress(AddressIndex _addressIndex, const QString& _label, const QString& _address) = 0;
    virtual void removeAddress(AddressIndex _addressIndex) = 0;

signals:
    void addressBookOpenedSignal();
    void addressBookClosedSignal();
    void addressAddedSignal(AddressIndex _addressIndex);
    void addressEditedSignal(AddressIndex _addressIndex);
//    void addressRemovedSignal(AddressIndex _addressIndex);
    void beginRemoveAddressSignal(AddressIndex _addressIndex);
    void endRemoveAddressSignal();
};

class AddressBookManager : public IAddressBookManager
{
    Q_OBJECT

public:
  AddressBookManager(QObject* _parent);
  virtual ~AddressBookManager() override;

  virtual AddressIndex getAddressCount() const override;
  virtual AddressItem getAddress(AddressIndex _addressIndex) const override;
  /*virtual*/ AddressIndex findAddressByAddress(const QString& _address) const /*override*/;
  /*virtual*/ AddressIndex findAddressByLabel(const QString& _label) const /*override*/;
  /*virtual*/ AddressIndex findAddress(const QString& _label, const QString& _address) const /*override*/;
  virtual void addAddress(const QString& _label, const QString& _address) override;
  virtual void editAddress(AddressIndex _addressIndex, const QString& _label, const QString& _address) override;
  virtual void removeAddress(AddressIndex _addressIndex) override;

private:
  QScopedPointer<QSettings> addressBook_;
  QHash<QString, AddressIndex> addressIndexes_;
  QHash<QString, AddressIndex> labelIndexes_;

  void buildIndexes();

//signals:
//  void addressBookOpenedSignal();
//  void addressBookClosedSignal();
//  void addressAddedSignal(AddressIndex _addressIndex);
//  void addressEditedSignal(AddressIndex _addressIndex);
////  void addressRemovedSignal(AddressIndex _addressIndex);
//  void beginRemoveAddressSignal(AddressIndex _addressIndex);
//  void endRemoveAddressSignal();
};

class MyAddressesManager : public IAddressBookManager
{
    Q_OBJECT

public:
    MyAddressesManager(QObject* _parent);
    virtual ~MyAddressesManager() override;

    virtual AddressIndex getAddressCount() const override;
    virtual AddressItem getAddress(AddressIndex _addressIndex) const override;
//    virtual AddressIndex findAddressByAddress(const QString& _address) const override;
//    virtual AddressIndex findAddressByLabel(const QString& _label) const override;
//    virtual AddressIndex findAddress(const QString& _label, const QString& _address) const override;
    virtual void addAddress(const QString& _label, const QString& _address) override;
    virtual void editAddress(AddressIndex _addressIndex, const QString& _label, const QString& _address) override;
    virtual void removeAddress(AddressIndex _addressIndex) override;

    /*virtual*/ AddressIndex findAddressByAddress(const QString& _address) const /*override*/;

    void requestAddresses(quint32 index = 0, quint32 count = std::numeric_limits<quint32>::max());
    void createAddress(const QString& label);
    void setAddressLabel(const QString& address, const QString& label);

    void connectedToWalletd();
    void disconnectedFromWalletd();

    void walletRecordsReceived(const RpcApi::WalletRecords& records);
    void addressLabelSetReceived(const QString& address, const QString& label);
    void addressCreatedReceived(const RpcApi::CreatedAddresses& addrs);

signals:
    void getWalletRecordsSignal(const RpcApi::GetWalletRecords::Request& req);
    void createAddressSignal(const QString& label);
    void setAddressLabelSignal(const RpcApi::SetAddressLabel::Request& req);

private:
    QList<AddressItem> addressBook_;
    QHash<QString, AddressIndex> addressIndexes_;
    bool firstTime_;

//    void buildIndexes();

};

}
