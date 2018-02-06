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
