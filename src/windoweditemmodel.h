// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#pragma once

#include <QSortFilterProxyModel>

namespace WalletGUI {

class WalletWindowedItemModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_DISABLE_COPY(WalletWindowedItemModel)

public:
    WalletWindowedItemModel(int filterRole, QObject* parent);
    virtual ~WalletWindowedItemModel();

    int getWindowSize() const;
    int getWindowBegin() const;
    void setWindowSize(int windowSize);
    void setWindowBegin(int windowBegin);

protected:
    virtual bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    int m_filterRole;
    int m_windowSize;
    int m_windowBegin;
};

}
