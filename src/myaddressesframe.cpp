// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QMessageBox>

#include "myaddressesframe.h"
#include "addressbookmanager.h"
#include "questiondialog.h"
#include "addressbookmodel.h"
#include "newmyaddressdialog.h"
#include "ui_myaddressesframe.h"
#include "mainwindow.h"

namespace WalletGUI
{

MyAddressesFrame::MyAddressesFrame(QWidget* parent)
    : QFrame(parent)
    , m_ui(new Ui::MyAddressesFrame)
//    , m_addressBookManager(nullptr)
    , mainWindow_(nullptr)
    , m_sortedAddressBookModel(nullptr)
{
    m_ui->setupUi(this);
}

MyAddressesFrame::~MyAddressesFrame() {
}

void MyAddressesFrame::setAddressBookManager(IAddressBookManager* manager)
{
    m_addressBookManager = manager;
}

void MyAddressesFrame::setMainWindow(MainWindow* _mainWindow)
{
  mainWindow_ = _mainWindow;
}

void MyAddressesFrame::setSortedAddressBookModel(QAbstractItemModel* _model) {
  m_sortedAddressBookModel = _model;
  m_ui->m_addressBookView->setModel(m_sortedAddressBookModel);
  m_ui->m_addressBookView->header()->setSectionResizeMode(AddressBookModel::COLUMN_LABEL, QHeaderView::Fixed);
  m_ui->m_addressBookView->header()->setSectionResizeMode(AddressBookModel::COLUMN_ADDRESS, QHeaderView::Stretch);
  m_ui->m_addressBookView->header()->setResizeContentsPrecision(-1);
  m_ui->m_addressBookView->header()->resizeSection(AddressBookModel::COLUMN_LABEL, 250);
}

void MyAddressesFrame::addClicked() {
    m_addressBookManager->addAddress(QString{}, QString{});
}

void MyAddressesFrame::editClicked(const QPersistentModelIndex& _index) {
    NewMyAddressDialog dlg{m_addressBookManager, _index, mainWindow_};
    if (dlg.exec() == QDialog::Accepted) {
        QString label = dlg.getLabel();
        m_addressBookManager->editAddress(_index.data(AddressBookModel::ROLE_ROW).toInt(), label, QString{});
    }
}

void MyAddressesFrame::contextMenu(const QPoint& _pos) {
  QPersistentModelIndex index = m_ui->m_addressBookView->indexAt(_pos);
  if (!index.isValid()) {
    return;
  }

  QMenu menu;
  menu.setObjectName("m_addressBookMenu");
  QAction* editAction = new QAction(tr("Edit"), &menu);
  QAction* copyAction = new QAction(tr("Copy to clipboard"), &menu);
  menu.addAction(editAction);
  menu.addAction(copyAction);

  connect(editAction, &QAction::triggered, [this, index]() {
      Q_EMIT this->editClicked(index);
    });
  connect(copyAction, &QAction::triggered, [index]() {
      QApplication::clipboard()->setText(index.data(AddressBookModel::ROLE_ADDRESS).toString());
    });

  menu.exec(m_ui->m_addressBookView->mapToGlobal(_pos) + QPoint(-10, 20));
}

}
