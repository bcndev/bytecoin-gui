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

#pragma once

#include <QFrame>

class QLabel;
class QPersistentModelIndex;
class QAbstractItemModel;

namespace Ui {
class AddressBookFrame;
}

namespace WalletGUI {

class AddressBookDelegate;
class MainWindow;
class AddressBookManager;

class AddressBookFrame : public QFrame
{
  Q_OBJECT
  Q_DISABLE_COPY(AddressBookFrame)

public:
  explicit AddressBookFrame(QWidget* _parent);
  ~AddressBookFrame();

  void setMainWindow(MainWindow* mainWindow);
  void setAddressBookManager(AddressBookManager* manager);
  void setSortedAddressBookModel(QAbstractItemModel* _model);

protected:
  void resizeEvent(QResizeEvent* _event) override;

private:
  QScopedPointer<Ui::AddressBookFrame> m_ui;
  AddressBookManager* m_addressBookManager;
  MainWindow* mainWindow_;
  QAbstractItemModel* m_sortedAddressBookModel;
  QLabel* m_helperLabel;
  AddressBookDelegate* m_addressBookDelegate;

  void rowsInserted(const QModelIndex& _parent, int _first, int _last);
  void rowsRemoved(const QModelIndex& _parent, int _first, int _last);
//  void sendToClicked(const QString& _address, const QString& label);

  Q_SLOT void addClicked();
  Q_SLOT void editClicked(const QPersistentModelIndex& _index);
  Q_SLOT void deleteClicked(const QPersistentModelIndex& _index);
  Q_SLOT void contextMenu(const QPoint& _pos);

Q_SIGNALS:
  void sendToSignal(const QString& _address, const QString& label);
};

}
