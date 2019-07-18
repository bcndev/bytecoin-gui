// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#ifndef WALLETMODEL
#define WALLETMODEL

#include <QAbstractItemModel>
#include <QPair>

#include "rpcapi.h"
#include "walletd.h"

namespace WalletGUI
{

struct WalletModelState;

class WalletModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY(WalletModel)
    Q_ENUMS(Columns)

public:
    enum Columns
    {
        COLUMN_FIRST_ADDRESS = 0, // getWalletInfo
        COLUMN_WALLET_TYPE,
        COLUMN_CAN_VIEW_OUTGOING_ADDRESSES,
        COLUMN_HAS_VIEW_SECRET_KEY,
        COLUMN_WALLET_CREATION_TIMESTAMP,
        COLUMN_TOTAL_ADDRESS_COUNT,
        COLUMN_NET,
        COLUMN_SECRET_VIEW_KEY,
        COLUMN_PUBLIC_VIEW_KEY,
        COLUMN_IMPORT_KEYS,
        COLUMN_MNEMONIC,
        COLUMN_VIEW_ONLY,

        COLUMN_UNLOCK_TIME, // listHistory (transactions)
        COLUMN_PAYMENT_ID,
        COLUMN_ANONYMITY,
        COLUMN_HASH,
        COLUMN_PREFIX_HASH,
        COLUMN_INPUTS_HASH,
        COLUMN_FEE,
        COLUMN_PK,
        COLUMN_EXTRA,
        COLUMN_COINBASE,
        COLUMN_AMOUNT,
        COLUMN_BLOCK_HEIGHT,
        COLUMN_BLOCK_HASH,
        COLUMN_TIMESTAMP,
        COLUMN_PROOF,

        COLUMN_TOP_BLOCK_HEIGHT, // getStatus
        COLUMN_TOP_BLOCK_TIMESTAMP,
        COLUMN_TOP_BLOCK_TIMESTAMP_MEDIAN,
        COLUMN_TOP_BLOCK_HASH,
        COLUMN_TOP_BLOCK_DIFFICULTY,
        COLUMN_NETWORK_HASHRATE,
        COLUMN_LOWER_LEVEL_ERROR,
        COLUMN_RECOMMENDED_MAX_TRANSACTION_SIZE,
        COLUMN_TXPOOL_VERSION,
        COLUMN_PEER_COUNT_OUTGOING,
        COLUMN_PEER_COUNT_INCOMING,
        COLUMN_RECOMMENDED_FEE_PER_BYTE,
        COLUMN_KNOWN_TOP_BLOCK_HEIGHT,
        COLUMN_PEER_COUNT_SUM,  // additional status

        COLUMN_STATE,

        COLUMN_SPENDABLE, // getBalance
        COLUMN_SPENDABLE_DUST,
        COLUMN_LOCKED_OR_UNCONFIRMED,
        COLUMN_SPENDABLE_OUTPUTS,
        COLUMN_SPENDABLE_DUST_OUTPUTS,
        COLUMN_LOCKED_OR_UNCONFIRMED_OUTPUTS,
        COLUMN_TOTAL,
    };

    enum Roles
    {
        ROLE_FIRST_ADDRESS = Qt::UserRole,
        ROLE_WALLET_TYPE,
        ROLE_CAN_VIEW_OUTGOING_ADDRESSES,
        ROLE_HAS_VIEW_SECRET_KEY,
        ROLE_WALLET_CREATION_TIMESTAMP,
        ROLE_TOTAL_ADDRESS_COUNT,
        ROLE_NET,
        ROLE_SECRET_VIEW_KEY,
        ROLE_PUBLIC_VIEW_KEY,
        ROLE_IMPORT_KEYS,
        ROLE_MNEMONIC,
        ROLE_VIEW_ONLY,

        ROLE_UNLOCK_TIME, // listHistory (transactions)
        ROLE_PAYMENT_ID,
        ROLE_ANONYMITY,
        ROLE_HASH,
        ROLE_PREFIX_HASH,
        ROLE_INPUTS_HASH,
        ROLE_FEE,
        ROLE_PK,
        ROLE_EXTRA,
        ROLE_COINBASE,
        ROLE_AMOUNT,
        ROLE_BLOCK_HEIGHT,
        ROLE_BLOCK_HASH,
        ROLE_TIMESTAMP,
        ROLE_RECIPIENTS,
        ROLE_PROOF,

        ROLE_TOP_BLOCK_HEIGHT, // getStatus
        ROLE_TOP_BLOCK_TIMESTAMP,
        ROLE_TOP_BLOCK_TIMESTAMP_MEDIAN,
        ROLE_TOP_BLOCK_HASH,
        ROLE_TOP_BLOCK_DIFFICULTY,
        ROLE_NETWORK_HASHRATE,
        ROLE_LOWER_LEVEL_ERROR,
        ROLE_RECOMMENDED_MAX_TRANSACTION_SIZE,
        ROLE_TXPOOL_VERSION,
        ROLE_PEER_COUNT_OUTGOING,
        ROLE_PEER_COUNT_INCOMING,
        ROLE_RECOMMENDED_FEE_PER_BYTE,
        ROLE_KNOWN_TOP_BLOCK_HEIGHT,
        ROLE_PEER_COUNT_SUM,

        ROLE_STATE,           // state

        ROLE_SPENDABLE, // getBalance
        ROLE_SPENDABLE_DUST,
        ROLE_LOCKED_OR_UNCONFIRMED,
        ROLE_SPENDABLE_OUTPUTS,
        ROLE_SPENDABLE_DUST_OUTPUTS,
        ROLE_LOCKED_OR_UNCONFIRMED_OUTPUTS,
        ROLE_TOTAL,
    };

    enum class SyncStatus
    {
        SYNCED,
        NOT_SYNCED,
        LAGGED,
    };

    WalletModel(QObject* parent);
    virtual ~WalletModel();

    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    virtual QModelIndex parent(const QModelIndex& index) const override;
    virtual bool canFetchMore(const QModelIndex& parent = QModelIndex{}) const override;
    virtual void fetchMore(const QModelIndex& parent = QModelIndex{}) override;

    void reset();

    QString getAddress() const;
    bool isConnected() const;
    bool isAmethyst() const;
    bool isViewOnly() const;

    quint32 getLastBlockHeight() const;
    QString getLastBlockHash() const;
    QDateTime getLastBlockTimestamp() const;
    quint64 getRecommendedFeePerByte() const;
    quint32 getKnownBlockHeight() const;
    quint32 getPeerCountOutgoing() const;
    quint32 getPeerCountIncoming() const;
    quint32 getPeerCountSum() const;
    QString getLowerLevelError() const;

    quint64 getSecsSinceLastBlock() const;
    QString getFormattedTimeSinceLastBlock() const;
    bool isThereAnyBlock() const;
    bool isSyncronized() const;
    quint32 getBlocksLeftToSync() const;
    SyncStatus getSyncStatus() const;

    int getTopFetchedHeight() const;
    int getBottomFetchedHeight() const;

signals:
    void getTransfersSignal(const RpcApi::GetTransfers::Request& req, RpcApi::Height topHeight);
    void fetchedSignal();
    void nothingToFetchSignal();
    void netChangedSignal(const QString& net);
    void statusUpdatedSignal();

public slots:
    void statusReceived(const RpcApi::Status& status);
    void transfersReceived(const RpcApi::Transfers& history, RpcApi::Height topHeight, RpcApi::Height from_height, RpcApi::Height to_height);
    void walletInfoReceived(const RpcApi::WalletInfo& info);
    void balanceReceived(const RpcApi::Balance& balance);

    void stateChanged(RemoteWalletd::State oldState, RemoteWalletd::State newState);

private:
    QVariant getEditRoleData(const QModelIndex& index) const;
    QVariant getDecorationRoleData(const QModelIndex& index) const;
    QVariant getToolTipRoleData(const QModelIndex& index, int role) const;
    QVariant getFontRoleData(const QModelIndex& index) const;

    QVariant getDisplayRoleData(const QModelIndex& index) const;

    QVariant getDisplayRoleAddresses(const QModelIndex& index) const;
    QVariant getDisplayRoleHistory(const QModelIndex& index) const;
    QVariant getDisplayRoleStatus(const QModelIndex& index) const;
    QVariant getDisplayRoleBalance(const QModelIndex& index) const;
    QVariant getDisplayRoleState(const QModelIndex& index) const;

    QVariant getUserRoleData(const QModelIndex& index, int role) const;

    QVariant getUserRoleAddresses(const QModelIndex& index, int role) const;
    QVariant getUserRoleHistory(const QModelIndex& index, int role) const;
    QVariant getUserRoleStatus(const QModelIndex& index, int role) const;
    QVariant getUserRoleBalance(const QModelIndex& index, int role) const;
    QVariant getUserRoleState(const QModelIndex& index, int role) const;

    template<typename Container>
    void containerReceived(Container& oldContainer, const Container& newContainer, int restSize);

    quint32 getTopConfirmedBlock() const;
    quint32 getBottomConfirmedBlock() const;
    quint32 getHighestKnownConfirmedBlock() const;

    const int columnCount_;
    QScopedPointer<WalletModelState> pimpl_;
};

}

#endif // WALLETMODEL

