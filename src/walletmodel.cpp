// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QLocale>
#include <QFont>
#include <QDateTime>
#include <QDebug>
#include <QMetaEnum>

#include "walletmodel.h"
#include "common.h"

#include "rpcapi.h"
#include "settings.h"

namespace WalletGUI
{

static const int SECS_IN_MINUTE = 60;
static const int SECS_IN_HOUR = 60 * SECS_IN_MINUTE;

class TxList
{
public:
    using Height = RpcApi::Height;
    using Transaction = RpcApi::Transaction;
    using Transfer = RpcApi::Transfer;
    using Map = QMultiMap<Height, Transaction>;
    using List = QList<Transaction>;

    TxList() {}
    void add(const Map& map, TxList::Height topHeight, TxList::Height from_height, TxList::Height to_height, TxList::Height next_from_height, TxList::Height next_to_height);
    const Map& map() const { return map_; }
    const List& list() const { return list_; }
    Height topHeight() const { return topHeight_; }
    int size() const { return map_.size(); }
    static Height confirmationThreshold(Height height) { return height > CONFIRMATIONS + 2 ? height - CONFIRMATIONS - 2 : 0; }

private:
    Map map_;
    List list_;
    Height topHeight_ = 0;

};

void TxList::add(const Map& newMap, TxList::Height /*topHeight*/, TxList::Height from_height, TxList::Height to_height, TxList::Height /*next_from_height*/, TxList::Height /*next_to_height*/)
{
    auto fhit = map_.upperBound(from_height);
    auto thit = map_.lowerBound(to_height);
    for (; fhit != thit;)
        fhit = map_.erase(fhit);

    map_.unite(newMap);
    list_ = map_.values();
}

struct WalletModelState
{
    RpcApi::Status status;
    RpcApi::Balance balance;
    RpcApi::Height prevTopHeight = 0;
    TxList txs;
    QList<QString> addresses;

    bool viewOnly = false;
    QString walletType;
    QDateTime creationTimestamp;
    quint32 addressesCount = 0;
    QString net{};

    RemoteWalletd::State walletdState = RemoteWalletd::State::STOPPED;
    int unconfirmedSize = 0;
    bool canFetchMore = true;
};

WalletModel::WalletModel(QObject* parent)
    : QAbstractItemModel(parent)
    , columnCount_(WalletModel::staticMetaObject.enumerator(WalletModel::staticMetaObject.indexOfEnumerator("Columns")).keyCount())
    , pimpl_(new WalletModelState)
{}

WalletModel::~WalletModel()
{}

Qt::ItemFlags WalletModel::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

int WalletModel::columnCount(const QModelIndex& /*parent = QModelIndex()*/) const
{
    return columnCount_;
}

int WalletModel::rowCount(const QModelIndex& /*parent = QModelIndex()*/) const
{
    const int size = qMax(pimpl_->addresses.size(), pimpl_->txs.size());
    return qMax(1, size);
}

QVariant WalletModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();

    switch(role)
    {
    case Qt::DisplayRole:
        switch(section)
        {
        case COLUMN_UNLOCK_TIME:
            return tr("Unlock time");
        case COLUMN_ANONYMITY:
            return tr("Anonymity");
        case COLUMN_EXTRA:
            return tr("Extra");
        case COLUMN_COINBASE:
            return tr("Base");
        case COLUMN_BLOCK_HASH:
            return tr("Block hash");
        case COLUMN_STATE:
            return tr("State");
        case COLUMN_TIMESTAMP:
            return tr("Date");
        case COLUMN_HASH:
            return tr("Tx hash");
        case COLUMN_AMOUNT:
            return tr("Amount");
        case COLUMN_PAYMENT_ID:
            return tr("Payment ID");
        case COLUMN_BLOCK_HEIGHT:
            return tr("Height");
        case COLUMN_FEE:
            return tr("Fee");
        case COLUMN_FIRST_ADDRESS:
            return tr("Address");
        case COLUMN_NET:
            return tr("Net");
        case COLUMN_PROOF:
            return tr("Proof");
        }
        break;
    case Qt::TextAlignmentRole:
    {
        return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
    }
//    case Qt::FontRole:
    }

    return QVariant();
}

QVariant WalletModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    if (!index.isValid())
        return QVariant();

    switch(role)
    {
    case Qt::DisplayRole:
        return getDisplayRoleData(index);
    case Qt::EditRole:
        return getEditRoleData(index);
    case Qt::DecorationRole:
        return getDecorationRoleData(index);
    case Qt::TextAlignmentRole:
        return headerData(index.column(), Qt::Horizontal, role);
    case Qt::ToolTipRole:
        return getToolTipRoleData(index, role);
    case Qt::FontRole:
        return getFontRoleData(index);
    default:
        return getUserRoleData(index, role);
    }

    return QVariant();
}

QModelIndex WalletModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    return createIndex(row, column);
}

QModelIndex WalletModel::parent(const QModelIndex& /*index*/) const
{
    return QModelIndex();
}

template<typename Container>
void WalletModel::containerReceived(Container& oldContainer, const Container& newContainer, int restSize)
{
    const int rows = rowCount();
    if (newContainer.size() < oldContainer.size())
    {
        const int sizeWithoutUs = qMax(restSize, 1);
        const int newSize = qMax(sizeWithoutUs, newContainer.size());

        if (newSize < rows)
        {
            beginRemoveRows(QModelIndex(), newSize, rows - 1);
            oldContainer = newContainer;
            endRemoveRows();
        }
        else
            oldContainer = newContainer;
    }
    else if (newContainer.size() > rows)
    {
        beginInsertRows(QModelIndex(), rows, newContainer.size() - 1);
        oldContainer = newContainer;
        endInsertRows();
    }
    else
    {
        oldContainer = newContainer;
    }
}

void WalletModel::walletInfoReceived(const RpcApi::WalletInfo& response)
{
    QList<QString> addresses;
    addresses.append(response.first_address);
    if (pimpl_->addresses == addresses && pimpl_->viewOnly == response.view_only)
        return;

    containerReceived(pimpl_->addresses, addresses, pimpl_->txs.size());
    pimpl_->viewOnly = response.view_only;
    pimpl_->addressesCount = response.total_address_count;
    pimpl_->creationTimestamp = response.wallet_creation_timestamp;
    pimpl_->walletType = response.wallet_type;
    if (pimpl_->net != response.net)
    {
        pimpl_->net = response.net;
        emit netChangedSignal(pimpl_->net);
    }

    QVector<int> changedAddressRoles;
    changedAddressRoles << Qt::EditRole << Qt::DisplayRole
        << ROLE_FIRST_ADDRESS
        << ROLE_WALLET_TYPE
        << ROLE_CAN_VIEW_OUTGOING_ADDRESSES
        << ROLE_HAS_VIEW_SECRET_KEY
        << ROLE_WALLET_CREATION_TIMESTAMP
        << ROLE_TOTAL_ADDRESS_COUNT
        << ROLE_NET
        << ROLE_SECRET_VIEW_KEY
        << ROLE_PUBLIC_VIEW_KEY
        << ROLE_IMPORT_KEYS
        << ROLE_MNEMONIC
        << ROLE_VIEW_ONLY;

    emit dataChanged(index(0, COLUMN_FIRST_ADDRESS), index(pimpl_->addresses.size() - 1, COLUMN_VIEW_ONLY), changedAddressRoles);
}

void WalletModel::transfersReceived(const RpcApi::Transfers& history, RpcApi::Height topHeight, RpcApi::Height from_height, RpcApi::Height to_height)
{
//    const RpcApi::Height confirmedThreshold = topHeight > CONFIRMATIONS + 2 ? topHeight - CONFIRMATIONS - 2 : 0;
//    const RpcApi::Height prevConfirmedThreshold = pimpl_->prevTopHeight > CONFIRMATIONS + 2 ? pimpl_->prevTopHeight - CONFIRMATIONS - 2 : 0;

    pimpl_->prevTopHeight = topHeight;

    TxList::Map rcvdTxs;
    for (const RpcApi::Block& block : history.blocks)
        for (const RpcApi::Transaction& tx : block.transactions)
            rcvdTxs.insert(tx.block_height, tx);

    if (!rcvdTxs.empty())
    {
        TxList newList = pimpl_->txs;
        newList.add(rcvdTxs, topHeight, from_height, to_height, history.next_from_height, history.next_to_height);
        containerReceived(pimpl_->txs, newList, pimpl_->addresses.size());
    }

    if (history.next_to_height == 0)
    {
        const bool cantFetchMore = (history.next_to_height == history.next_from_height);
        pimpl_->canFetchMore = !cantFetchMore;
    }
    if (pimpl_->canFetchMore)
        emit fetchedSignal();
    else
        emit nothingToFetchSignal();


//    if (from_height >= pimpl_->prevTopHeight && history.next_from_height < )

//    for (auto it = pimpl_->txs.begin(); it != pimpl_->txs.end() && it->block_height > confirmedThreshold; it = pimpl_->txs.erase(it))
//        ;

//    for (const RpcApi::Transaction& tx : pimpl_->txs)
//        if (tx.block_height < )

//    if (history.next_from_height)





//    const quint32 highestConfirmedBlock = getHighestKnownConfirmedBlock();
//    if (history.next_from_height >= highestConfirmedBlock) // unconfirmed
//    {
////        QLinkedList<RpcApi::Transaction> rcvdTxs;
//        QList<RpcApi::Transaction> rcvdTxs;
//        for (const RpcApi::Block& block : history.blocks)
//            rcvdTxs.append(block.transactions);

//        if (rcvdTxs == pimpl_->txs.mid(0, pimpl_->unconfirmedSize))
//            return;

//        const QList<RpcApi::Transaction>& confirmedTxs = pimpl_->txs.mid(pimpl_->unconfirmedSize);
//        pimpl_->unconfirmedSize = rcvdTxs.size();
//        rcvdTxs.append(confirmedTxs);
//        containerReceived(pimpl_->txs, rcvdTxs, pimpl_->addresses.size());
//    }
//    else if (history.next_to_height < highestConfirmedBlock) // confirmed
//    {
//        QList<RpcApi::Transaction> rcvdTxs;
//        for (const RpcApi::Block& block : history.blocks)
//            rcvdTxs.append(block.transactions);

//        if (rcvdTxs.empty())
//            return;
//        if (rcvdTxs.first().block_height < getBottomConfirmedBlock())
//        {
////            QList<RpcApi::Transaction> newTxs = pimpl_->txs;
////            newTxs.append(rcvdTxs);

//            beginInsertRows(QModelIndex(), rowCount(), pimpl_->txs.size() + rcvdTxs.size() - 1);
//            pimpl_->txs.append(std::move(rcvdTxs));
////            oldContainer = newContainer;
//            endInsertRows();

////            containerReceived(pimpl_->txs, newTxs, pimpl_->addresses.size());
//            pimpl_->canFetchMore = history.next_to_height != 0;
//            if (pimpl_->canFetchMore)
//                emit fetchedSignal();
//            else
//                emit nothingToFetchSignal();
//        }
//        else if (rcvdTxs.last().block_height > getTopConfirmedBlock())
//        {
//            QList<RpcApi::Transaction> newTxs = pimpl_->txs.mid(0, pimpl_->unconfirmedSize);
//            newTxs.append(rcvdTxs);
//            newTxs.append(pimpl_->txs.mid(pimpl_->unconfirmedSize));
//            containerReceived(pimpl_->txs, newTxs, pimpl_->addresses.size());
//        }
//    }
//    else
//    {
//        qDebug("Got %d block, but %d expected", history.next_from_height, highestConfirmedBlock);
//    }

    QVector<int> changedRoles;
    changedRoles << Qt::EditRole << Qt::DisplayRole
        << ROLE_UNLOCK_TIME
        << ROLE_PAYMENT_ID
        << ROLE_ANONYMITY
        << ROLE_HASH
        << ROLE_FEE
        << ROLE_PK
        << ROLE_EXTRA
        << COLUMN_COINBASE
        << ROLE_AMOUNT
        << ROLE_BLOCK_HEIGHT
        << ROLE_BLOCK_HASH
//        << ROLE_TRANSFER_COUNT
//        << ROLE_STATE
        << ROLE_TIMESTAMP;

    emit dataChanged(index(0, COLUMN_UNLOCK_TIME), index(pimpl_->txs.size() - 1, COLUMN_TIMESTAMP), changedRoles);
}

void WalletModel::statusReceived(const RpcApi::Status& status)
{
    if (status == pimpl_->status)
        return;

//    bool needToRequestUnconfirmed = false;
//    bool needToRequestConfirmed = false;

    QVector<int> changedRoles;
    if (status.top_block_hash != pimpl_->status.top_block_hash)
    {
        changedRoles << ROLE_TOP_BLOCK_HASH;
//        needToRequestConfirmed = needToRequestUnconfirmed = true;
    }
    if (status.outgoing_peer_count != pimpl_->status.outgoing_peer_count)
        changedRoles << ROLE_PEER_COUNT_OUTGOING << ROLE_PEER_COUNT_SUM;
    if (status.incoming_peer_count != pimpl_->status.incoming_peer_count)
        changedRoles << ROLE_PEER_COUNT_INCOMING << ROLE_PEER_COUNT_SUM;
    if (status.top_block_height != pimpl_->status.top_block_height)
        changedRoles << ROLE_TOP_BLOCK_HEIGHT;
    if (status.top_block_difficulty != pimpl_->status.top_block_difficulty)
        changedRoles << ROLE_TOP_BLOCK_DIFFICULTY << ROLE_NETWORK_HASHRATE;
    if (status.lower_level_error != pimpl_->status.lower_level_error)
        changedRoles << ROLE_LOWER_LEVEL_ERROR;
    if (status.top_block_timestamp != pimpl_->status.top_block_timestamp)
        changedRoles << ROLE_TOP_BLOCK_TIMESTAMP;
    if (status.top_block_timestamp_median != pimpl_->status.top_block_timestamp_median)
        changedRoles << ROLE_TOP_BLOCK_TIMESTAMP_MEDIAN;
    if (status.recommended_max_transaction_size != pimpl_->status.recommended_max_transaction_size)
        changedRoles << ROLE_RECOMMENDED_MAX_TRANSACTION_SIZE;
    if (status.recommended_fee_per_byte != pimpl_->status.recommended_fee_per_byte)
        changedRoles << ROLE_RECOMMENDED_FEE_PER_BYTE;
    if (status.top_known_block_height != pimpl_->status.top_known_block_height)
    {
        changedRoles << ROLE_KNOWN_TOP_BLOCK_HEIGHT;
//        needToRequestConfirmed = needToRequestUnconfirmed = true;
    }
    if (status.transaction_pool_version != pimpl_->status.transaction_pool_version)
    {
        changedRoles << ROLE_TXPOOL_VERSION;
//        needToRequestUnconfirmed = true;
    }

    pimpl_->status = status;
    changedRoles << Qt::EditRole << Qt::DisplayRole;

    emit statusUpdatedSignal();
    emit dataChanged(index(0, COLUMN_TOP_BLOCK_HEIGHT), index(0, COLUMN_PEER_COUNT_SUM), changedRoles);

    const bool firstRequest = pimpl_->prevTopHeight == 0;

    RpcApi::GetTransfers::Request req;
    req.from_height = TxList::confirmationThreshold(pimpl_->prevTopHeight);
    req.to_height = std::numeric_limits<RpcApi::Height>::max();
    req.desired_transactions_count = firstRequest ? 300 : std::numeric_limits<RpcApi::Height>::max();
    req.forward = false;
    emit getTransfersSignal(req, status.top_block_height);


//    if (needToRequestConfirmed)
//    {
//        RpcApi::GetTransfers::Request req;
//        req.from_height = getTopConfirmedBlock();
//        req.to_height = getHighestKnownConfirmedBlock();
//        emit getTransfersSignal(req, 0);
//    }

//    if (needToRequestUnconfirmed)
//    {
//        RpcApi::GetTransfers::Request req;
//        req.from_height = getHighestKnownConfirmedBlock();
//        emit getTransfersSignal(req, 0);
//    }
}

//quint32 WalletModel::getTopConfirmedBlock() const
//{
//    return pimpl_->unconfirmedSize < pimpl_->txs.size() ? pimpl_->txs[pimpl_->unconfirmedSize].block_height : 0;
//}

quint32 WalletModel::getBottomConfirmedBlock() const
{
    return pimpl_->unconfirmedSize < pimpl_->txs.size() ? pimpl_->txs.map().first().block_height : std::numeric_limits<quint32>::max();
}

quint32 WalletModel::getHighestKnownConfirmedBlock() const
{
//    const quint32 topKnownBlockHeight = pimpl_->status.top_known_block_height;
    const quint32 topKnownBlockHeight = pimpl_->status.top_block_height;
    return topKnownBlockHeight < CONFIRMATIONS + 2 ? 0 : topKnownBlockHeight - CONFIRMATIONS - 2;
}

void WalletModel::balanceReceived(const RpcApi::Balance& balance)
{
    if (balance == pimpl_->balance)
        return;
    pimpl_->balance = balance;

    QVector<int> changedRoles;
    changedRoles << Qt::EditRole << Qt::DisplayRole
        << ROLE_SPENDABLE
        << ROLE_SPENDABLE_DUST
        << ROLE_LOCKED_OR_UNCONFIRMED
        << ROLE_SPENDABLE_OUTPUTS
        << ROLE_SPENDABLE_DUST_OUTPUTS
        << ROLE_LOCKED_OR_UNCONFIRMED_OUTPUTS
        << ROLE_TOTAL;

//    qDebug("[WalletModel] Balance changed.\n\tSpendable: %s\n\tSpendable dust: %s\n\tLocked or unconfirmed: %s\n\tTotal: %s",
//           qPrintable(formatUnsignedAmount(balance.spendable)),
//           qPrintable(formatUnsignedAmount(balance.spendable_dust)),
//           qPrintable(formatUnsignedAmount(balance.locked_or_unconfirmed)),
//           qPrintable(formatUnsignedAmount(balance.spendable + balance.spendable_dust + balance.locked_or_unconfirmed)));
    emit dataChanged(index(0, COLUMN_SPENDABLE), index(0, COLUMN_TOTAL), changedRoles);
}

void WalletModel::stateChanged(RemoteWalletd::State /*oldState*/, RemoteWalletd::State newState)
{
    if (newState == pimpl_->walletdState)
        return;
    pimpl_->walletdState = newState;

    QVector<int> changedRoles;
    changedRoles << Qt::EditRole << Qt::DisplayRole
        << ROLE_STATE;

    emit dataChanged(index(0, COLUMN_STATE), index(0, COLUMN_STATE), changedRoles);
}

QVariant WalletModel::getDisplayRoleData(const QModelIndex& index) const
{
    if (index.column() >= COLUMN_STATE && index.column() <= COLUMN_STATE)
        return getDisplayRoleState(index);

    if (index.column() >= COLUMN_FIRST_ADDRESS && index.column() <= COLUMN_VIEW_ONLY)
        return getDisplayRoleAddresses(index);

    if (index.column() >= COLUMN_UNLOCK_TIME && index.column() <= COLUMN_PROOF)
        return getDisplayRoleHistory(index);

    if (index.column() >= COLUMN_TOP_BLOCK_HEIGHT && index.column() <= COLUMN_PEER_COUNT_SUM)
        return getDisplayRoleStatus(index);

    if (index.column() >= COLUMN_SPENDABLE && index.column() <= COLUMN_TOTAL)
        return getDisplayRoleBalance(index);

    return QVariant();
}

QVariant WalletModel::getUserRoleData(const QModelIndex& index, int role) const
{
    if (role >= ROLE_STATE && role <= ROLE_STATE)
        return getUserRoleState(index, role);

    if (role >= ROLE_FIRST_ADDRESS && role <= ROLE_VIEW_ONLY)
        return getUserRoleAddresses(index, role);

    if (role >= ROLE_UNLOCK_TIME && role <= ROLE_PROOF)
        return getUserRoleHistory(index, role);

    if (role >= ROLE_TOP_BLOCK_HEIGHT && role <= ROLE_PEER_COUNT_SUM)
        return getUserRoleStatus(index, role);

    if (role >= ROLE_SPENDABLE && role <= ROLE_TOTAL)
        return getUserRoleBalance(index, role);

    return QVariant();
}

QVariant WalletModel::getDisplayRoleState(const QModelIndex& index) const
{
    switch(index.column())
    {
    case COLUMN_STATE:
    {
        switch(pimpl_->walletdState)
        {
        case RemoteWalletd::State::STOPPED:
            return tr("Disconnected.");
        case RemoteWalletd::State::CONNECTING:
            return tr("Connecting to %1.").arg(Settings::instance().getUserFriendlyWalletdConnectionMethod());
        case RemoteWalletd::State::CONNECTED:
            return tr("Connected to %1.").arg(Settings::instance().getUserFriendlyWalletdConnectionMethod());
        case RemoteWalletd::State::NETWORK_ERROR:
            return tr("Network error.");
        case RemoteWalletd::State::JSON_ERROR:
            return tr("RPC API error.");
        }
    }
    }
    return QVariant();
}

QVariant WalletModel::getUserRoleState(const QModelIndex& /*index*/, int role) const
{
    switch(role)
    {
    case ROLE_STATE:
        return static_cast<int>(pimpl_->walletdState);
    }
    return QVariant();
}


QVariant WalletModel::getDisplayRoleAddresses(const QModelIndex& index) const
{
    if (index.row() >= pimpl_->addresses.size())
        return QVariant();

    switch(index.column())
    {
    case COLUMN_FIRST_ADDRESS:
        return pimpl_->addresses[index.row()];
    case COLUMN_WALLET_TYPE:
        return (pimpl_->walletType == "amethyst") ? QVariant{"<b>"+tr("HD")+"</b>"} : (pimpl_->walletType == "hardware") ? QVariant{"<b>"+tr("HW")+"</b>"} : QVariant{};
    case COLUMN_WALLET_CREATION_TIMESTAMP:
        return tr("Wallet created: %1.").arg(pimpl_->creationTimestamp.toString(Qt::SystemLocaleShortDate));
    case COLUMN_TOTAL_ADDRESS_COUNT:
        return tr("Total addresses in the wallet: %1.").arg(pimpl_->addressesCount);
    case COLUMN_NET:
        return pimpl_->net;
    case COLUMN_VIEW_ONLY:
        return pimpl_->viewOnly ? tr("The wallet is view-only.") : QString{};
    }

    return QVariant();
}

QVariant WalletModel::getUserRoleAddresses(const QModelIndex& index, int role) const
{
    if (index.row() >= pimpl_->addresses.size())
        return QVariant();

    switch (role)
    {
    case ROLE_FIRST_ADDRESS:
        return pimpl_->addresses[index.row()];
    case ROLE_WALLET_CREATION_TIMESTAMP:
        return pimpl_->creationTimestamp;
    case ROLE_TOTAL_ADDRESS_COUNT:
        return pimpl_->addressesCount;
    case ROLE_NET:
        return pimpl_->net;
    case ROLE_VIEW_ONLY:
        return pimpl_->viewOnly;
    }

    return QVariant();
}

QVariant WalletModel::getDisplayRoleHistory(const QModelIndex& index) const
{
    const int size = pimpl_->txs.size();
    const int row = index.row();
    if (row >= size)
        return QVariant();
//    const RpcApi::Transaction& tx = pimpl_->txs[index.row()];
//    const RpcApi::Transaction tx = pimpl_->txs.map().values().at(size - row - 1);
    const RpcApi::Transaction& tx = pimpl_->txs.list()[size - row - 1];

    switch(index.column())
    {
    case COLUMN_UNLOCK_TIME:
    {
        if (tx.unlock_block_or_timestamp == 0)
            return QVariant();
        if (isTransactionSpendTimeUnlocked(tx.unlock_block_or_timestamp, pimpl_->status.top_block_height, pimpl_->status.top_block_timestamp_median.toTime_t()))
            return tr("Unlocked");
        if (tx.unlock_block_or_timestamp < CRYPTONOTE_MAX_BLOCK_NUMBER)
            return tr("Locked till %1 block").arg(tx.unlock_block_or_timestamp);
        return tr("Locked till %1").arg(tx.timestamp.toString(Qt::SystemLocaleShortDate));
    }
    case COLUMN_PAYMENT_ID:
    {
        if (tx.payment_id.isEmpty())
            return QVariant();
        return tx.payment_id;
    }
    case COLUMN_ANONYMITY:
        return tx.anonymity;
    case COLUMN_HASH:
        return tx.hash;
    case COLUMN_FEE:
        return formatAmount(tx.fee);
    case COLUMN_PK:
        return tx.public_key;
    case COLUMN_EXTRA:
        return tx.extra;
    case COLUMN_COINBASE:
        return tx.coinbase;
    case COLUMN_AMOUNT:
    {
        bool isOur = false;
        quint64 amount = 0;
        for (const RpcApi::Transfer& tr : tx.transfers)
        {
            if (tr.ours)
            {
                isOur = true;
                amount += tr.amount;
            }
        }
        return isOur ? QVariant(formatAmount(amount)) : 0;
    }
    case COLUMN_BLOCK_HEIGHT:
    {
//        if (tx.block_height > getLastBlockHeight())
        if (tx.block_hash.isEmpty())
            return tr("-", "n/a");
        return tx.block_height;
    }
    case COLUMN_BLOCK_HASH:
    {
//        if (tx.block_height > getLastBlockHeight())
        if (tx.block_hash.isEmpty())
            return tr("In mempool");
        return tx.block_hash;
    }
    case COLUMN_TIMESTAMP:
    {
        if (tx.timestamp.isNull())
            return tr("Unknown");
        return tx.timestamp.toLocalTime().toString(Qt::SystemLocaleShortDate);
    }
    case COLUMN_PROOF:
    {
        bool proof = false;
        for (const RpcApi::Transfer& tr : tx.transfers)
        {
            if (!tr.ours)
            {
                proof = true;
                break;
            }
        }
        return pimpl_->viewOnly ? QVariant{} : proof ? QVariant(tr("Get")) : QVariant(/*tr("Try")*/);
    }
    }

    return QVariant();
}

QVariant WalletModel::getUserRoleHistory(const QModelIndex& index, int role) const
{
    const int size = pimpl_->txs.size();
    const int row = index.row();
    if (row >= size)
        return QVariant();
//    const RpcApi::Transaction& tx = pimpl_->txs[index.row()];
//    const RpcApi::Transaction tx = pimpl_->txs.map().values().at(size - row - 1);
    const RpcApi::Transaction& tx = pimpl_->txs.list()[size - row - 1];

    switch (role)
    {
    case ROLE_UNLOCK_TIME:
        return tx.unlock_block_or_timestamp;
    case ROLE_PAYMENT_ID:
        return tx.payment_id;
    case ROLE_ANONYMITY:
        return tx.anonymity;
    case ROLE_HASH:
        return tx.hash;
    case ROLE_RECIPIENTS:
    {
        QStringList addresses;
        for (const RpcApi::Transfer& tr : tx.transfers)
        {
            if (!tr.ours)
                addresses << tr.address;
        }
        return addresses;
    }
    case ROLE_FEE:
        return tx.fee;
    case ROLE_PK:
        return tx.public_key;
    case ROLE_EXTRA:
        return tx.extra;
    case ROLE_COINBASE:
        return tx.coinbase;
    case ROLE_AMOUNT:
    {
        quint64 amount = 0;
        for (const RpcApi::Transfer& tr : tx.transfers)
        {
            if (tr.ours)
                amount += tr.amount;
        }
        return amount;
    }
    case ROLE_BLOCK_HEIGHT:
        return tx.block_height;
    case ROLE_BLOCK_HASH:
        return tx.block_hash;
    case ROLE_TIMESTAMP:
        return tx.timestamp;
    case ROLE_PROOF:
    {
        bool proof = false;
        for (const RpcApi::Transfer& tr : tx.transfers)
        {
            if (!tr.ours)
                proof = true;
        }
        return proof;
    }
    }

    return QVariant();
}

QVariant WalletModel::getDisplayRoleStatus(const QModelIndex& index) const
{
    switch(index.column())
    {
    case COLUMN_TOP_BLOCK_HEIGHT:
        return pimpl_->status.top_block_height;
    case COLUMN_TOP_BLOCK_TIMESTAMP:
    {
        if (pimpl_->status.top_block_timestamp.isNull())
            return tr("Unknown");
        return pimpl_->status.top_block_timestamp.toString(Qt::SystemLocaleShortDate);
    }
    case COLUMN_TOP_BLOCK_TIMESTAMP_MEDIAN:
    {
        if (pimpl_->status.top_block_timestamp_median.isNull())
            return tr("Unknown");
        return pimpl_->status.top_block_timestamp_median.toString(Qt::SystemLocaleShortDate);
    }
    case COLUMN_TOP_BLOCK_HASH:
        return pimpl_->status.top_block_hash;
    case COLUMN_TOP_BLOCK_DIFFICULTY:
    {
        if (pimpl_->status.top_block_height < pimpl_->status.top_known_block_height)
            return tr("-", "n/a");
        return pimpl_->status.top_block_difficulty;
    }
    case COLUMN_NETWORK_HASHRATE:
    {
        if (pimpl_->status.top_block_height < pimpl_->status.top_known_block_height)
            return tr("-", "n/a");
        return formatHashRate(pimpl_->status.top_block_difficulty / DIFFICULTY_TARGET);
    }
    case COLUMN_LOWER_LEVEL_ERROR:
//        return pimpl_->status.lower_level_error;
        return pimpl_->status.lower_level_error.isEmpty() || (pimpl_->status.top_block_height < pimpl_->status.top_known_block_height && pimpl_->status.lower_level_error == "SEND_ERROR") ? tr("Connected to bytecoind.") : tr("Bytecoind status: %1.").arg(pimpl_->status.lower_level_error);
    case COLUMN_RECOMMENDED_MAX_TRANSACTION_SIZE:
        return pimpl_->status.recommended_max_transaction_size;
    case COLUMN_TXPOOL_VERSION:
        return pimpl_->status.transaction_pool_version;
    case COLUMN_PEER_COUNT_OUTGOING:
        return tr("Outgoing: %1 peer(s)").arg(pimpl_->status.outgoing_peer_count);
    case COLUMN_PEER_COUNT_INCOMING:
        return tr("Incoming: %1 peer(s)").arg(pimpl_->status.incoming_peer_count);
    case COLUMN_RECOMMENDED_FEE_PER_BYTE:
        return tr("Recommended fee per byte: %1").arg(pimpl_->status.recommended_fee_per_byte);
    case COLUMN_KNOWN_TOP_BLOCK_HEIGHT:
        return pimpl_->status.top_known_block_height;
    case COLUMN_PEER_COUNT_SUM:
        return tr("%1 peer(s).").arg(pimpl_->status.outgoing_peer_count + pimpl_->status.incoming_peer_count);
    }

    return QVariant();
}

QVariant WalletModel::getUserRoleStatus(const QModelIndex& /*index*/, int role) const
{
    switch (role)
    {
    case ROLE_TOP_BLOCK_HEIGHT:
        return pimpl_->status.top_block_height;
    case ROLE_TOP_BLOCK_TIMESTAMP:
        return pimpl_->status.top_block_timestamp;
    case ROLE_TOP_BLOCK_TIMESTAMP_MEDIAN:
        return pimpl_->status.top_block_timestamp_median;
    case ROLE_TOP_BLOCK_HASH:
        return pimpl_->status.top_block_hash;
    case ROLE_TOP_BLOCK_DIFFICULTY:
        return pimpl_->status.top_block_difficulty;
    case ROLE_NETWORK_HASHRATE:
        return pimpl_->status.top_block_difficulty / DIFFICULTY_TARGET;
    case ROLE_LOWER_LEVEL_ERROR:
        return pimpl_->status.lower_level_error;
    case ROLE_RECOMMENDED_MAX_TRANSACTION_SIZE:
        return pimpl_->status.recommended_max_transaction_size;
    case ROLE_TXPOOL_VERSION:
        return pimpl_->status.transaction_pool_version;
    case ROLE_PEER_COUNT_OUTGOING:
        return pimpl_->status.outgoing_peer_count;
    case ROLE_PEER_COUNT_INCOMING:
        return pimpl_->status.incoming_peer_count;
    case ROLE_RECOMMENDED_FEE_PER_BYTE:
        return pimpl_->status.recommended_fee_per_byte;
    case ROLE_KNOWN_TOP_BLOCK_HEIGHT:
        return pimpl_->status.top_known_block_height;
    case ROLE_PEER_COUNT_SUM:
        return pimpl_->status.incoming_peer_count + pimpl_->status.outgoing_peer_count;
    }

    return QVariant();
}

QVariant WalletModel::getDisplayRoleBalance(const QModelIndex& index) const
{
    switch(index.column())
    {
    case COLUMN_SPENDABLE:
        return formatUnsignedAmount(pimpl_->balance.spendable);
    case COLUMN_SPENDABLE_DUST:
        return formatUnsignedAmount(pimpl_->balance.spendable_dust);
    case COLUMN_LOCKED_OR_UNCONFIRMED:
        return formatUnsignedAmount(pimpl_->balance.locked_or_unconfirmed);
    case COLUMN_TOTAL:
        return formatUnsignedAmount(pimpl_->balance.spendable + pimpl_->balance.spendable_dust + pimpl_->balance.locked_or_unconfirmed);
    }

    return QVariant();
}

QVariant WalletModel::getUserRoleBalance(const QModelIndex& /*index*/, int role) const
{
    switch(role)
    {
    case ROLE_SPENDABLE:
        return pimpl_->balance.spendable;
    case ROLE_SPENDABLE_DUST:
        return pimpl_->balance.spendable_dust;
    case ROLE_LOCKED_OR_UNCONFIRMED:
        return pimpl_->balance.locked_or_unconfirmed;
    case ROLE_TOTAL:
        return pimpl_->balance.spendable + pimpl_->balance.spendable_dust + pimpl_->balance.locked_or_unconfirmed;
    }

    return QVariant();
}

QVariant WalletModel::getToolTipRoleData(const QModelIndex& /*index*/, int role) const
{
    switch(role)
    {
    case ROLE_WALLET_TYPE:
        return (pimpl_->walletType == "amethyst") ? QVariant{"Amethyst wallet"} : (pimpl_->walletType == "hardware") ? QVariant{"Hardware wallet"} : QVariant{};
    }

    return QVariant();
}

QVariant WalletModel::getFontRoleData(const QModelIndex& /*index*/) const
{
    return QVariant();
}

QVariant WalletModel::getEditRoleData(const QModelIndex& index) const
{
    return getDisplayRoleData(index);
}

QVariant WalletModel::getDecorationRoleData(const QModelIndex& /*index*/) const
{
//    if (index.column() == COLUMN_STATE || index.column() == COLUMN_AMOUNT)
//    {
//        TransactionType transactionType = static_cast<TransactionType>(_index.data(ROLE_TYPE).value<quint8>());
//        quint32 transactionConfirmationCount = index.data(ROLE_NUMBER_OF_CONFIRMATIONS).value<quint32>();
//        QString iconPrefix = getIconPrefix(transactionType);
//        Q_ASSERT(!iconPrefix.isEmpty());
//        CryptoNote::WalletTransactionState transactionState = static_cast<CryptoNote::WalletTransactionState>(_index.data(ROLE_STATE).value<quint8>());
//        QString file;
//        if (transactionState == CryptoNote::WalletTransactionState::FAILED)
//            file = ":icons/out_failed";
//        else if (transactionType == TransactionType::FUSION)
//            file = QString(":icons/%1confirmed").arg(iconPrefix);
//        else
//        {
//            if (transactionConfirmationCount == 0)
//                file = QString(":icons/%1unconfirmed").arg(iconPrefix);
//            else if (transactionConfirmationCount < 2)
//                file = QString(":icons/%1clock1").arg(iconPrefix);
//            else if (transactionConfirmationCount < 4)
//                file = QString(":icons/%1clock2").arg(iconPrefix);
//            else if (transactionConfirmationCount < 6)
//                file = QString(":icons/%1clock3").arg(iconPrefix);
//            else if (transactionConfirmationCount < 8)
//                file = QString(":icons/%1clock4").arg(iconPrefix);
//            else if (transactionConfirmationCount < 10)
//                file = QString(":icons/%1clock5").arg(iconPrefix);
//            else
//                file = QString(":icons/%1confirmed").arg(iconPrefix);
//        }

//        QPixmap pixmap;
//        if (!QPixmapCache::find(file, pixmap))
//        {
//            pixmap.load(file);
//            QPixmapCache::insert(file, pixmap);
//        }

//        return pixmap;
//    }

    return QVariant();
}

quint32 WalletModel::getLastBlockHeight() const
{
    return pimpl_->status.top_block_height;
}

QString WalletModel::getLastBlockHash() const
{
    return pimpl_->status.top_block_hash;
}

QDateTime WalletModel::getLastBlockTimestamp() const
{
    return pimpl_->status.top_block_timestamp;
}

quint64 WalletModel::getRecommendedFeePerByte() const
{
    return pimpl_->status.recommended_fee_per_byte;
}

quint32 WalletModel::getKnownBlockHeight() const
{
    return pimpl_->status.top_known_block_height;
}

quint32 WalletModel::getPeerCountOutgoing() const
{
    return pimpl_->status.outgoing_peer_count;
}

quint32 WalletModel::getPeerCountIncoming() const
{
    return pimpl_->status.incoming_peer_count;
}

quint32 WalletModel::getPeerCountSum() const
{
    return pimpl_->status.incoming_peer_count + pimpl_->status.outgoing_peer_count;
}

QString WalletModel::getLowerLevelError() const
{
    return pimpl_->status.lower_level_error;
}

quint64 WalletModel::getSecsSinceLastBlock() const
{
    const QDateTime currentDateTime = QDateTime::currentDateTimeUtc();
    const QDateTime lastBlockTimestamp = qMin(getLastBlockTimestamp(), currentDateTime);
    return lastBlockTimestamp.secsTo(currentDateTime);
}

QString WalletModel::getFormattedTimeSinceLastBlock() const
{
    return isThereAnyBlock() ? formatTimeDiff(getSecsSinceLastBlock()) : tr("unknown");
}

bool WalletModel::isThereAnyBlock() const
{
    return getLastBlockTimestamp().toMSecsSinceEpoch() > 0;
}

bool WalletModel::isSyncronized() const
{
    return getLowerLevelError().isEmpty() && isThereAnyBlock() && (getLastBlockHeight() == getKnownBlockHeight());
}

quint32 WalletModel::getBlocksLeftToSync() const
{
    return getKnownBlockHeight() >= getLastBlockHeight() ? (getKnownBlockHeight() - getLastBlockHeight()) : 0;
}

WalletModel::SyncStatus WalletModel::getSyncStatus() const
{
    const WalletModel::SyncStatus result = !isSyncronized() ? WalletModel::SyncStatus::NOT_SYNCED
                                                            : getSecsSinceLastBlock() > SECS_IN_HOUR ? WalletModel::SyncStatus::LAGGED
                                                                                                     : WalletModel::SyncStatus::SYNCED;
    return result;
}

QString WalletModel::getAddress() const
{
    if (pimpl_->addresses.isEmpty())
        return QString();
    return pimpl_->addresses.first();
}

bool WalletModel::isConnected() const
{
    return pimpl_->walletdState == RemoteWalletd::State::CONNECTED;
}

bool WalletModel::isAmethyst() const
{
    return pimpl_->walletType == "amethyst" || pimpl_->walletType == "hardware";
}

bool WalletModel::isViewOnly() const
{
    return pimpl_->viewOnly;
}

void WalletModel::reset()
{
    beginResetModel();
    pimpl_.reset(new WalletModelState);
    endResetModel();


    QVector<int> changedRoles;
    changedRoles << Qt::EditRole << Qt::DisplayRole
        << ROLE_UNLOCK_TIME
        << ROLE_PAYMENT_ID
        << ROLE_ANONYMITY
        << ROLE_HASH
        << ROLE_FEE
        << ROLE_PK
        << ROLE_EXTRA
        << ROLE_COINBASE
        << ROLE_AMOUNT
        << ROLE_BLOCK_HEIGHT
        << ROLE_BLOCK_HASH
        << ROLE_TIMESTAMP
        << ROLE_PROOF

        << ROLE_TOP_BLOCK_HEIGHT
        << ROLE_TOP_BLOCK_TIMESTAMP
        << ROLE_TOP_BLOCK_TIMESTAMP_MEDIAN
        << ROLE_TOP_BLOCK_HASH
        << ROLE_TOP_BLOCK_DIFFICULTY
        << ROLE_NETWORK_HASHRATE
        << ROLE_LOWER_LEVEL_ERROR
        << ROLE_RECOMMENDED_MAX_TRANSACTION_SIZE
        << ROLE_TXPOOL_VERSION
        << ROLE_PEER_COUNT_OUTGOING
        << ROLE_PEER_COUNT_INCOMING
        << ROLE_RECOMMENDED_FEE_PER_BYTE
        << ROLE_KNOWN_TOP_BLOCK_HEIGHT
        << ROLE_PEER_COUNT_SUM

        << ROLE_STATE

        << ROLE_SPENDABLE
        << ROLE_SPENDABLE_DUST
        << ROLE_LOCKED_OR_UNCONFIRMED
        << ROLE_SPENDABLE_OUTPUTS
        << ROLE_SPENDABLE_DUST_OUTPUTS
        << ROLE_LOCKED_OR_UNCONFIRMED_OUTPUTS
        << ROLE_TOTAL;

    emit dataChanged(index(0, COLUMN_UNLOCK_TIME), index(0, COLUMN_TOTAL), changedRoles);
}

void WalletModel::fetchMore(const QModelIndex& parent)
{
    if (parent.isValid())
        return;
//    RpcApi::GetTransfers::Request req;
//    req.to_height = getBottomConfirmedBlock() - 1;
//    emit getTransfersSignal(req, 0);

    RpcApi::GetTransfers::Request req;
    req.from_height = 0;
    req.to_height = getBottomConfirmedBlock() - 1;
    req.desired_transactions_count = 300;
    req.forward = false;
    emit getTransfersSignal(req, pimpl_->status.top_block_height);

}

bool WalletModel::canFetchMore(const QModelIndex& parent) const
{
    return !parent.isValid() && pimpl_->canFetchMore;
}

//int WalletModel::getTopFetchedHeight() const
//{
//    return getTopConfirmedBlock();
//}

int WalletModel::getBottomFetchedHeight() const
{
    return canFetchMore() ? getBottomConfirmedBlock() : 0;
}


}
