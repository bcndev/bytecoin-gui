// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include "addressbookdialog.h"
#include "addressbookmodel.h"

#include "ui_addressbookdialog.h"

namespace WalletGUI {

AddressBookDialog::AddressBookDialog(QAbstractItemModel* _addressBookModel, QWidget* _parent) :
  QDialog(_parent, static_cast<Qt::WindowFlags>(Qt::WindowCloseButtonHint)), m_ui(new Ui::AddressBookDialog) {
  m_ui->setupUi(this);
  m_ui->m_addressBookView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  m_ui->m_addressBookView->setModel(_addressBookModel);
//  m_ui->m_addressBookView->setItemDelegateForColumn(AddressBookModel::COLUMN_ADDRESS, new RightAlignmentColumnDelegate(true, this));
  m_ui->m_addressBookView->header()->setSectionResizeMode(AddressBookModel::COLUMN_LABEL, QHeaderView::Fixed);
  m_ui->m_addressBookView->header()->setSectionResizeMode(AddressBookModel::COLUMN_ADDRESS, QHeaderView::Stretch);
//  m_ui->m_addressBookView->header()->hideSection(AddressBookModel::COLUMN_DONATION);
  m_ui->m_addressBookView->header()->resizeSection(AddressBookModel::COLUMN_LABEL, 200);
  if (_addressBookModel->rowCount() > 0) {
    m_ui->m_addressBookView->setCurrentIndex(_addressBookModel->index(0, 0));
  }
}

AddressBookDialog::~AddressBookDialog() {
}

QString AddressBookDialog::getAddress() const {
  return m_ui->m_addressBookView->currentIndex().data(AddressBookModel::ROLE_ADDRESS).toString();
}

QString AddressBookDialog::getLabel() const {
  return m_ui->m_addressBookView->currentIndex().data(AddressBookModel::ROLE_LABEL).toString();
}

}
