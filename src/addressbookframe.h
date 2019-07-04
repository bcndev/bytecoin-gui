// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

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
