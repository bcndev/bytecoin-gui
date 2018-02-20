#include <QLocale>
#include <QFont>
#include <QDateTime>
#include <QDebug>
#include <QMetaEnum>

#include "walletmodel.h"
#include "common.h"

#include "rpcapi.h" // ??
#include "settings.h"

namespace WalletGUI
{

struct WalletModelState
{
    RpcApi::Status status;
    RpcApi::Balance balance;
    QList<RpcApi::Transaction> txs;
    QList<QString> addresses;

    RemoteWalletd::State walletdState = RemoteWalletd::State::STOPPED;
    int unconfimedSize = 0;
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
        case COLUMN_ADDRESS:
            return tr("Address");
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
        return getToolTipRoleData(index);
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

void WalletModel::addressesReceived(const RpcApi::Addresses& response)
{
    const QList<QString>& addresses = response.addresses;
    if (pimpl_->addresses == addresses)
        return;

    containerReceived(pimpl_->addresses, addresses, pimpl_->txs.size());

    QVector<int> changedAddressRoles;
    changedAddressRoles << Qt::EditRole << Qt::DisplayRole
        << ROLE_ADDRESS;

    emit dataChanged(index(0, COLUMN_ADDRESS), index(pimpl_->addresses.size() - 1, COLUMN_ADDRESS), changedAddressRoles);
}

void WalletModel::transfersReceived(const RpcApi::Transfers& history)
{
    const quint32 highestConfirmedBlock = pimpl_->status.top_known_block_height - CONFIRMATIONS - 2;
    if (history.next_from_height >= highestConfirmedBlock)
    {
        QList<RpcApi::Transaction> txs;
        for (const RpcApi::Block& block : history.blocks)
            txs.append(block.transactions);

        if (txs == pimpl_->txs.mid(0, pimpl_->unconfimedSize))
            return;

        const QList<RpcApi::Transaction>& confirmedTxs = pimpl_->txs.mid(pimpl_->unconfimedSize);
        pimpl_->unconfimedSize = txs.size();
        txs.append(confirmedTxs);
        containerReceived(pimpl_->txs, txs, pimpl_->addresses.size());
    }
    else if (history.next_to_height < highestConfirmedBlock)
    {
        QList<RpcApi::Transaction> txs;
        for (const RpcApi::Block& block : history.blocks)
            txs.append(block.transactions);

        if (txs.empty())
            return;
        if (txs.first().block_height < getBottomConfirmedBlock())
        {
            QList<RpcApi::Transaction> newTxs = pimpl_->txs;
            newTxs.append(txs);
            containerReceived(pimpl_->txs, newTxs, pimpl_->addresses.size());
            pimpl_->canFetchMore = history.next_to_height != 0;
        }
        else if (txs.last().block_height > getTopConfirmedBlock())
        {
            QList<RpcApi::Transaction> newTxs = pimpl_->txs.mid(0, pimpl_->unconfimedSize);
            newTxs.append(txs);
            newTxs.append(pimpl_->txs.mid(pimpl_->unconfimedSize));
            containerReceived(pimpl_->txs, newTxs, pimpl_->addresses.size());
        }
    }
    else
    {
        qDebug("Got %d block, but %d expected", history.next_from_height, highestConfirmedBlock);
    }

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

    bool needToRequestUnconfirmed = false;
    bool needToRequestConfirmed = false;

    QVector<int> changedRoles;
    if (status.top_block_hash != pimpl_->status.top_block_hash)
    {
        changedRoles << ROLE_TOP_BLOCK_HASH;
        needToRequestConfirmed = needToRequestUnconfirmed = true;
    }
    if (status.outgoing_peer_count != pimpl_->status.outgoing_peer_count)
        changedRoles << ROLE_PEER_COUNT_OUTGOING << ROLE_PEER_COUNT_SUM;
    if (status.incoming_peer_count != pimpl_->status.incoming_peer_count)
        changedRoles << ROLE_PEER_COUNT_INCOMING << ROLE_PEER_COUNT_SUM;
    if (status.top_block_height != pimpl_->status.top_block_height)
        changedRoles << ROLE_TOP_BLOCK_HEIGHT;
    if (status.top_block_difficulty != pimpl_->status.top_block_difficulty)
        changedRoles << ROLE_TOP_BLOCK_DIFFICULTY << ROLE_NETWORK_HASHRATE;
    if (status.top_block_timestamp != pimpl_->status.top_block_timestamp)
        changedRoles << ROLE_TOP_BLOCK_TIMESTAMP;
    if (status.top_block_timestamp_median != pimpl_->status.top_block_timestamp_median)
        changedRoles << ROLE_TOP_BLOCK_TIMESTAMP_MEDIAN;
    if (status.next_block_effective_median_size != pimpl_->status.next_block_effective_median_size)
        changedRoles << ROLE_NEXT_BLOCK_EFFECTIVE_MEDIAN_SIZE;
    if (status.recommended_fee_per_byte != pimpl_->status.recommended_fee_per_byte)
        changedRoles << ROLE_RECOMMENDED_FEE_PER_BYTE;
    if (status.top_known_block_height != pimpl_->status.top_known_block_height)
    {
        changedRoles << ROLE_KNOWN_TOP_BLOCK_HEIGHT;
        needToRequestConfirmed = needToRequestUnconfirmed = true;
    }
    if (status.transaction_pool_version != pimpl_->status.transaction_pool_version)
    {
        changedRoles << ROLE_TXPOOL_VERSION;
        needToRequestUnconfirmed = true;
    }

    pimpl_->status = status;
    changedRoles << Qt::EditRole << Qt::DisplayRole;

    emit dataChanged(index(0, COLUMN_TOP_BLOCK_HEIGHT), index(0, COLUMN_PEER_COUNT_SUM), changedRoles);

    const quint32 highestConfirmedBlock = pimpl_->status.top_known_block_height - CONFIRMATIONS - 2;
    if (needToRequestConfirmed)
    {
        RpcApi::GetTransfers::Request req;
        req.from_height = getTopConfirmedBlock();
        req.to_height = highestConfirmedBlock;
        emit getTransfersSignal(req);
    }

    if (needToRequestUnconfirmed)
    {
        RpcApi::GetTransfers::Request req;
        req.from_height = highestConfirmedBlock;
        emit getTransfersSignal(req);
    }
}

quint32 WalletModel::getTopConfirmedBlock() const
{
    return pimpl_->unconfimedSize < pimpl_->txs.size() ? pimpl_->txs[pimpl_->unconfimedSize].block_height : 0;
}

quint32 WalletModel::getBottomConfirmedBlock() const
{
    return pimpl_->unconfimedSize < pimpl_->txs.size() ? pimpl_->txs.last().block_height : std::numeric_limits<quint32>::max();
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
        << ROLE_TOTAL;

    qDebug("[WalletModel] Balance changed.\n\tSpendable: %s\n\tSpendable dust: %s\n\tLocked or unconfirmed: %s\n\tTotal: %s",
           qPrintable(formatUnsignedAmount(balance.spendable)),
           qPrintable(formatUnsignedAmount(balance.spendable_dust)),
           qPrintable(formatUnsignedAmount(balance.locked_or_unconfirmed)),
           qPrintable(formatUnsignedAmount(balance.spendable + balance.spendable_dust + balance.locked_or_unconfirmed)));
    emit dataChanged(index(0, COLUMN_SPENDABLE), index(0, COLUMN_TOTAL), changedRoles);
}

void WalletModel::viewKeyReceived(const RpcApi::ViewKey& /*viewKey*/)
{
    // TODO
}

void WalletModel::unspentsReceived(const RpcApi::Unspents& /*unspents*/)
{
    // we do not use this api
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

    if (index.column() >= COLUMN_ADDRESS && index.column() <= COLUMN_ADDRESS)
        return getDisplayRoleAddresses(index);

    if (index.column() >= COLUMN_UNLOCK_TIME && index.column() <= COLUMN_TIMESTAMP)
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

    if (role >= ROLE_ADDRESS && role <= ROLE_ADDRESS)
        return getUserRoleAddresses(index, role);

    if (role >= ROLE_UNLOCK_TIME && role <= ROLE_TIMESTAMP)
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
            return tr("Disconnected");
        case RemoteWalletd::State::CONNECTING:
            return tr("Connecting to %1").arg(Settings::instance().getUserFriendlyConnectionMethod());
        case RemoteWalletd::State::CONNECTED:
            return tr("Connected to %1").arg(Settings::instance().getUserFriendlyConnectionMethod());
        case RemoteWalletd::State::NETWORK_ERROR:
            return tr("Network error");
        case RemoteWalletd::State::JSON_ERROR:
            return tr("RPC API error");
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
    case COLUMN_ADDRESS:
        return pimpl_->addresses[index.row()];
    }

    return QVariant();
}

QVariant WalletModel::getUserRoleAddresses(const QModelIndex& index, int role) const
{
    if (index.row() >= pimpl_->addresses.size())
        return QVariant();

    switch (role)
    {
    case ROLE_ADDRESS:
        return pimpl_->addresses[index.row()];
    }

    return QVariant();
}

QVariant WalletModel::getDisplayRoleHistory(const QModelIndex& index) const
{
    if (index.row() >= pimpl_->txs.size())
        return QVariant();
    const RpcApi::Transaction& tx = pimpl_->txs[index.row()];

    switch(index.column())
    {
    case COLUMN_UNLOCK_TIME:
    {
        if (tx.unlock_time == 0)
            return QVariant();
        if (isTransactionSpendTimeUnlocked(tx.unlock_time, tx.block_height, pimpl_->status.top_block_timestamp_median.toTime_t()))
            return tr("Unlocked");
        if (tx.unlock_time < CRYPTONOTE_MAX_BLOCK_NUMBER)
            return tr("Locked till %1 block").arg(tx.unlock_time);
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
        return isOur ? QVariant(formatAmount(amount)) : QVariant();
    }
    case COLUMN_BLOCK_HEIGHT:
    {
        if (tx.block_height > getLastBlockHeight())
            return tr("-", "n/a");
        return tx.block_height;
    }
    case COLUMN_BLOCK_HASH:
    {
        if (tx.block_height > getLastBlockHeight())
            return tr("In mempool");
        return tx.block_hash;
    }
    case COLUMN_TIMESTAMP:
    {
        if (tx.timestamp.isNull())
            return tr("Unknown");
        return tx.timestamp.toString(Qt::SystemLocaleShortDate);
    }
    }

    return QVariant();
}

QVariant WalletModel::getUserRoleHistory(const QModelIndex& index, int role) const
{
    if (index.row() >= pimpl_->txs.size())
        return QVariant();
    const RpcApi::Transaction& tx = pimpl_->txs[index.row()];

    switch (role)
    {
    case ROLE_UNLOCK_TIME:
        return tx.unlock_time;
    case ROLE_PAYMENT_ID:
        return tx.payment_id;
    case ROLE_ANONYMITY:
        return tx.anonymity;
    case ROLE_HASH:
        return tx.hash;
    case ROLE_FEE:
        return tx.fee;
    case ROLE_PK:
        return tx.public_key;
    case ROLE_EXTRA:
        return tx.extra;
    case COLUMN_COINBASE:
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
    case COLUMN_NEXT_BLOCK_EFFECTIVE_MEDIAN_SIZE:
        return pimpl_->status.next_block_effective_median_size;
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
        return tr("%1 peer(s)").arg(pimpl_->status.outgoing_peer_count + pimpl_->status.incoming_peer_count);
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
    case ROLE_NEXT_BLOCK_EFFECTIVE_MEDIAN_SIZE:
        return pimpl_->status.next_block_effective_median_size;
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

QVariant WalletModel::getToolTipRoleData(const QModelIndex& /*index*/) const
{
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

QString WalletModel::getAddress() const
{
    if (pimpl_->addresses.isEmpty())
        return QString();
    return pimpl_->addresses.first();
}

void WalletModel::reset()
{
    beginResetModel();
    pimpl_.reset(new WalletModelState);
    endResetModel();
}

void WalletModel::fetchMore(const QModelIndex& parent)
{
    if (parent.isValid())
        return;
    RpcApi::GetTransfers::Request req;
    req.to_height = getBottomConfirmedBlock() - 1;
    emit getTransfersSignal(req);
}

bool WalletModel::canFetchMore(const QModelIndex& parent) const
{
    return !parent.isValid() && pimpl_->canFetchMore;
}

}
