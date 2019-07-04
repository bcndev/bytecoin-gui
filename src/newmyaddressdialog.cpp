// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QStyle>

#include "newmyaddressdialog.h"
#include "addressbookmodel.h"
#include "addressbookmanager.h"
#include "ui_newmyaddressdialog.h"

namespace WalletGUI
{

NewMyAddressDialog::NewMyAddressDialog(IAddressBookManager* addressBookManager, QWidget* _parent)
    : QDialog(_parent, static_cast<Qt::WindowFlags>(Qt::WindowCloseButtonHint))
    , m_ui(new Ui::NewMyAddressDialog)
    , m_addressBookManager(addressBookManager)
{
  m_ui->setupUi(this);
}

NewMyAddressDialog::NewMyAddressDialog(IAddressBookManager* addressBookManager, const QPersistentModelIndex& _index, QWidget* _parent)
    : NewMyAddressDialog(addressBookManager, _parent)
{
  setWindowTitle(tr("Edit address"));
  m_ui->m_labelEdit->setText(_index.data(AddressBookModel::ROLE_LABEL).toString());
  m_ui->m_addressLabel->setText(_index.data(AddressBookModel::ROLE_ADDRESS).toString());
}

NewMyAddressDialog::~NewMyAddressDialog() {
}

QString NewMyAddressDialog::getAddress() const {
  return m_ui->m_addressLabel->text();
}

QString NewMyAddressDialog::getLabel() const {
  return m_ui->m_labelEdit->text();
}

//void NewMyAddressDialog::setAddressError(bool _error) {
//  m_ui->m_addressEdit->setProperty("errorState", _error);
//  m_ui->m_addressTextLabel->setProperty("errorState", _error);
//  m_ui->m_addressTextLabel->setText(_error ? tr("INVALID ADDRESS") : tr("ADDRESS"));

//  m_ui->m_addressEdit->style()->unpolish(m_ui->m_addressEdit);
//  m_ui->m_addressEdit->style()->polish(m_ui->m_addressEdit);
//  m_ui->m_addressEdit->update();

//  m_ui->m_addressTextLabel->style()->unpolish(m_ui->m_addressTextLabel);
//  m_ui->m_addressTextLabel->style()->polish(m_ui->m_addressTextLabel);
//  m_ui->m_addressTextLabel->update();
//  m_ui->m_okButton->setEnabled(!checkForErrors() && !m_ui->m_labelEdit->text().trimmed().isEmpty() &&
//    !m_ui->m_addressEdit->text().trimmed().isEmpty());
//}

//void NewMyAddressDialog::setAddressDuplicationError(bool _error) {
//  m_ui->m_addressEdit->setProperty("errorState", _error);
//  m_ui->m_addressTextLabel->setProperty("errorState", _error);
//  m_ui->m_addressTextLabel->setText(_error ? tr("ALREADY IN THE ADDRESS BOOK") : tr("ADDRESS"));

//  m_ui->m_addressEdit->style()->unpolish(m_ui->m_addressEdit);
//  m_ui->m_addressEdit->style()->polish(m_ui->m_addressEdit);
//  m_ui->m_addressEdit->update();

//  m_ui->m_addressTextLabel->style()->unpolish(m_ui->m_addressTextLabel);
//  m_ui->m_addressTextLabel->style()->polish(m_ui->m_addressTextLabel);
//  m_ui->m_addressTextLabel->update();

//  m_ui->m_okButton->setEnabled(!checkForErrors() && !m_ui->m_labelEdit->text().trimmed().isEmpty() &&
//     !m_ui->m_addressEdit->text().trimmed().isEmpty());
//}

//void NewMyAddressDialog::setLabelDuplicationError(bool _error) {
//  m_ui->m_labelEdit->setProperty("errorState", _error);
//  m_ui->m_labelTextLabel->setProperty("errorState", _error);
//  m_ui->m_labelTextLabel->setText(_error ? tr("ALREADY IN THE ADDRESS BOOK") : tr("LABEL"));

//  m_ui->m_labelEdit->style()->unpolish(m_ui->m_labelEdit);
//  m_ui->m_labelEdit->style()->polish(m_ui->m_labelEdit);
//  m_ui->m_labelEdit->update();

//  m_ui->m_labelTextLabel->style()->unpolish(m_ui->m_labelTextLabel);
//  m_ui->m_labelTextLabel->style()->polish(m_ui->m_labelTextLabel);
//  m_ui->m_labelTextLabel->update();

//  m_ui->m_okButton->setEnabled(!checkForErrors() && !m_ui->m_labelEdit->text().trimmed().isEmpty() &&
//     !m_ui->m_addressEdit->text().trimmed().isEmpty());
//}

//bool NewMyAddressDialog::checkForErrors() const {
//  return m_ui->m_addressEdit->property("errorState").toBool() || m_ui->m_labelEdit->property("errorState").toBool();
//}

//void NewMyAddressDialog::validateAddress(const QString& _address) {
//  bool isInvalidAddress = (/*!*/_address.isEmpty()/* && !m_cryptoNoteAdapter->isValidAddress(_address)*/);
//  if (isInvalidAddress) {
//    setAddressError(true);
//    return;
//  }

//  QString address = getAddress().trimmed();
//  quintptr addressIndex = m_addressBookManager->findAddressByAddress(address);
//  setAddressDuplicationError(addressIndex != INVALID_ADDRESS_INDEX);
//}

//void NewMyAddressDialog::validateLabel(const QString& /*_label*/) {
//  QString label = getLabel().trimmed();
//  quintptr addressIndex = m_addressBookManager->findAddressByLabel(label);
//  setLabelDuplicationError(addressIndex != INVALID_ADDRESS_INDEX);
//}

}
