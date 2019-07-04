// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QFrame>

class QLabel;
class QPersistentModelIndex;
class QAbstractItemModel;

namespace Ui {
class MyAddressesFrame;
}

namespace WalletGUI {

class MainWindow;
class IAddressBookManager;

class MyAddressesFrame : public QFrame
{
  Q_OBJECT
  Q_DISABLE_COPY(MyAddressesFrame)

public:
  explicit MyAddressesFrame(QWidget* _parent);
  ~MyAddressesFrame();

  void setMainWindow(MainWindow* mainWindow);
  void setAddressBookManager(IAddressBookManager* manager);
  void setSortedAddressBookModel(QAbstractItemModel* _model);

private:
  QScopedPointer<Ui::MyAddressesFrame> m_ui;
  IAddressBookManager* m_addressBookManager;
  MainWindow* mainWindow_;
  QAbstractItemModel* m_sortedAddressBookModel;

  Q_SLOT void addClicked();
  Q_SLOT void editClicked(const QPersistentModelIndex& _index);
  Q_SLOT void contextMenu(const QPoint& _pos);
};

}
