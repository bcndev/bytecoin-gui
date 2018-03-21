// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include "windoweditemmodel.h"

namespace WalletGUI {

WalletWindowedItemModel::WalletWindowedItemModel(int filterRole, QObject* parent)
    : QSortFilterProxyModel(parent)
    , m_filterRole(filterRole)
    , m_windowSize(0)
    , m_windowBegin(0)
{}

WalletWindowedItemModel::~WalletWindowedItemModel()
{}

int WalletWindowedItemModel::getWindowSize() const
{
    return m_windowSize;
}

int WalletWindowedItemModel::getWindowBegin() const
{
    return m_windowBegin;
}

void WalletWindowedItemModel::setWindowSize(int windowSize)
{
    if (m_windowSize != windowSize)
    {
        m_windowSize = windowSize;
        invalidateFilter();
    }
}

void WalletWindowedItemModel::setWindowBegin(int windowBegin)
{
    if (m_windowBegin != windowBegin)
    {
        m_windowBegin = windowBegin;
        invalidateFilter();
    }
}

bool WalletWindowedItemModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
    if (getWindowSize() == 0)
        return false;

    if (m_filterRole == -1)
        return sourceRow >= m_windowBegin && sourceRow < m_windowBegin + m_windowSize;

    QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    return sourceIndex.data(m_filterRole).toInt() >= m_windowBegin &&
        sourceIndex.data(m_filterRole).toInt() < m_windowBegin + m_windowSize;
}

}
