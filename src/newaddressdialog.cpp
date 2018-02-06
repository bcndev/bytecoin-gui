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

#include "newaddressdialog.h"
#include "addressbookmodel.h"
#include "addressbookmanager.h"
#include "ui_newaddressdialog.h"

namespace WalletGUI
{

NewAddressDialog::NewAddressDialog(AddressBookManager* addressBookManager, QWidget* _parent)
    : QDialog(_parent, static_cast<Qt::WindowFlags>(Qt::WindowCloseButtonHint))
    , m_ui(new Ui::NewAddressDialog)
    , m_addressBookManager(addressBookManager)
{
  m_ui->setupUi(this);
}

NewAddressDialog::NewAddressDialog(AddressBookManager* addressBookManager, const QPersistentModelIndex& _index, QWidget* _parent)
    : NewAddressDialog(addressBookManager, _parent)
{
  setWindowTitle(tr("Edit address"));
  m_ui->m_labelEdit->setText(_index.data(AddressBookModel::ROLE_LABEL).toString());
  m_ui->m_addressEdit->setText(_index.data(AddressBookModel::ROLE_ADDRESS).toString());
  setAddressDuplicationError(false);
  setLabelDuplicationError(false);
}

NewAddressDialog::~NewAddressDialog() {
}

QString NewAddressDialog::getAddress() const {
  return m_ui->m_addressEdit->text();
}

QString NewAddressDialog::getLabel() const {
  return m_ui->m_labelEdit->text();
}

void NewAddressDialog::setAddressError(bool _error) {
  m_ui->m_addressEdit->setProperty("errorState", _error);
  m_ui->m_addressTextLabel->setProperty("errorState", _error);
  m_ui->m_addressTextLabel->setText(_error ? tr("INVALID ADDRESS") : tr("ADDRESS"));

  m_ui->m_addressEdit->style()->unpolish(m_ui->m_addressEdit);
  m_ui->m_addressEdit->style()->polish(m_ui->m_addressEdit);
  m_ui->m_addressEdit->update();

  m_ui->m_addressTextLabel->style()->unpolish(m_ui->m_addressTextLabel);
  m_ui->m_addressTextLabel->style()->polish(m_ui->m_addressTextLabel);
  m_ui->m_addressTextLabel->update();
  m_ui->m_okButton->setEnabled(!checkForErrors() && !m_ui->m_labelEdit->text().trimmed().isEmpty() &&
    !m_ui->m_addressEdit->text().trimmed().isEmpty());
}

void NewAddressDialog::setAddressDuplicationError(bool _error) {
  m_ui->m_addressEdit->setProperty("errorState", _error);
  m_ui->m_addressTextLabel->setProperty("errorState", _error);
  m_ui->m_addressTextLabel->setText(_error ? tr("ALREADY IN THE ADDRESS BOOK") : tr("ADDRESS"));

  m_ui->m_addressEdit->style()->unpolish(m_ui->m_addressEdit);
  m_ui->m_addressEdit->style()->polish(m_ui->m_addressEdit);
  m_ui->m_addressEdit->update();

  m_ui->m_addressTextLabel->style()->unpolish(m_ui->m_addressTextLabel);
  m_ui->m_addressTextLabel->style()->polish(m_ui->m_addressTextLabel);
  m_ui->m_addressTextLabel->update();

  m_ui->m_okButton->setEnabled(!checkForErrors() && !m_ui->m_labelEdit->text().trimmed().isEmpty() &&
     !m_ui->m_addressEdit->text().trimmed().isEmpty());
}

void NewAddressDialog::setLabelDuplicationError(bool _error) {
  m_ui->m_labelEdit->setProperty("errorState", _error);
  m_ui->m_labelTextLabel->setProperty("errorState", _error);
  m_ui->m_labelTextLabel->setText(_error ? tr("ALREADY IN THE ADDRESS BOOK") : tr("LABEL"));

  m_ui->m_labelEdit->style()->unpolish(m_ui->m_labelEdit);
  m_ui->m_labelEdit->style()->polish(m_ui->m_labelEdit);
  m_ui->m_labelEdit->update();

  m_ui->m_labelTextLabel->style()->unpolish(m_ui->m_labelTextLabel);
  m_ui->m_labelTextLabel->style()->polish(m_ui->m_labelTextLabel);
  m_ui->m_labelTextLabel->update();

  m_ui->m_okButton->setEnabled(!checkForErrors() && !m_ui->m_labelEdit->text().trimmed().isEmpty() &&
     !m_ui->m_addressEdit->text().trimmed().isEmpty());
}

bool NewAddressDialog::checkForErrors() const {
  return m_ui->m_addressEdit->property("errorState").toBool() || m_ui->m_labelEdit->property("errorState").toBool();
}

void NewAddressDialog::validateAddress(const QString& _address) {
  bool isInvalidAddress = (/*!*/_address.isEmpty()/* && !m_cryptoNoteAdapter->isValidAddress(_address)*/);
  if (isInvalidAddress) {
    setAddressError(true);
    return;
  }

  QString address = getAddress().trimmed();
  quintptr addressIndex = m_addressBookManager->findAddressByAddress(address);
  setAddressDuplicationError(addressIndex != INVALID_ADDRESS_INDEX);
}

void NewAddressDialog::validateLabel(const QString& /*_label*/) {
  QString label = getLabel().trimmed();
  quintptr addressIndex = m_addressBookManager->findAddressByLabel(label);
  setLabelDuplicationError(addressIndex != INVALID_ADDRESS_INDEX);
}

}
