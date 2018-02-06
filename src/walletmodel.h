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
        COLUMN_ADDRESS = 0, // getAddresses

        COLUMN_UNLOCK_TIME, // listHistory (transactions)
        COLUMN_PAYMENT_ID,
        COLUMN_ANONYMITY,
        COLUMN_HASH,
        COLUMN_FEE,
        COLUMN_PK,
        COLUMN_EXTRA,
        COLUMN_COINBASE,
        COLUMN_AMOUNT,
        COLUMN_BLOCK_HEIGHT,
        COLUMN_BLOCK_HASH,
        COLUMN_TIMESTAMP,

        COLUMN_TOP_BLOCK_HEIGHT, // getStatus
        COLUMN_TOP_BLOCK_TIMESTAMP,
        COLUMN_TOP_BLOCK_TIMESTAMP_MEDIAN,
        COLUMN_TOP_BLOCK_HASH,
        COLUMN_TOP_BLOCK_DIFFICULTY,
        COLUMN_NETWORK_HASHRATE,
        COLUMN_NEXT_BLOCK_EFFECTIVE_MEDIAN_SIZE,
        COLUMN_TXPOOL_VERSION,
        COLUMN_PEER_COUNT_OUTGOING,
        COLUMN_PEER_COUNT_INCOMING,
        COLUMN_RECOMMENDED_FEE_PER_BYTE,
        COLUMN_KNOWN_TOP_BLOCK_HEIGHT,
        COLUMN_PEER_COUNT_SUM,  // additional status

//        COLUMN_WALLET_CONNECT,
        COLUMN_STATE,

        COLUMN_SPENDABLE, // getBalance
        COLUMN_SPENDABLE_DUST,
        COLUMN_LOCKED_OR_UNCONFIRMED,
        COLUMN_TOTAL,

//        COLUMN_PUBLIC_KEY,
//        COLUMN_GLOBAL_INDEX,
//        COLUMN_AMOUNT,
//        COLUMN_UNLOCK_TIME,
//        COLUMN_HEIGHT,
//        COLUMN_KEY_IMAGE,

//        COLUMN_VIEW_SECRET_KEY, // getViewKey
//        COLUMN_VIEW_PUBLIC_KEY,
    };

    enum Roles
    {
        ROLE_ADDRESS = Qt::UserRole,

        ROLE_UNLOCK_TIME, // listHistory (transactions)
        ROLE_PAYMENT_ID,
        ROLE_ANONYMITY,
        ROLE_HASH,
        ROLE_FEE,
        ROLE_PK,
        ROLE_EXTRA,
        ROLE_COINBASE,
        ROLE_AMOUNT,
        ROLE_BLOCK_HEIGHT,
        ROLE_BLOCK_HASH,
        ROLE_TIMESTAMP,

        ROLE_TOP_BLOCK_HEIGHT, // getStatus
        ROLE_TOP_BLOCK_TIMESTAMP,
        ROLE_TOP_BLOCK_TIMESTAMP_MEDIAN,
        ROLE_TOP_BLOCK_HASH,
        ROLE_TOP_BLOCK_DIFFICULTY,
        ROLE_NETWORK_HASHRATE,
        ROLE_NEXT_BLOCK_EFFECTIVE_MEDIAN_SIZE,
        ROLE_TXPOOL_VERSION,
        ROLE_PEER_COUNT_OUTGOING,
        ROLE_PEER_COUNT_INCOMING,
        ROLE_RECOMMENDED_FEE_PER_BYTE,
        ROLE_KNOWN_TOP_BLOCK_HEIGHT,
        ROLE_PEER_COUNT_SUM,

        ROLE_STATE,           // state
//        ROLE_WALLET_CONNECT,

        ROLE_SPENDABLE, // getBalance
        ROLE_SPENDABLE_DUST,
        ROLE_LOCKED_OR_UNCONFIRMED,
        ROLE_TOTAL,

//        ROLE_PUBLIC_KEY,
//        ROLE_GLOBAL_INDEX,
//        ROLE_AMOUNT,
//        ROLE_UNLOCK_TIME,
//        ROLE_HEIGHT,
//        ROLE_KEY_IMAGE,

//        ROLE_VIEW_SECRET_KEY, // getViewKey
//        ROLE_VIEW_PUBLIC_KEY,
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
    virtual bool canFetchMore(const QModelIndex& parent) const override;
    virtual void fetchMore(const QModelIndex& parent) override;

//    bool isInitialized() const;
    void reset();

//    bool isConnected() const;
//    bool hasAddress() const;

    QString getAddress() const;

//    QVariant getAddress(int index = 0) const;
    quint32 getLastBlockHeight() const;
    QString getLastBlockHash() const;
    QDateTime getLastBlockTimestamp() const;
    quint64 getRecommendedFeePerByte() const;
    quint32 getKnownBlockHeight() const;
    quint32 getPeerCountOutgoing() const;
    quint32 getPeerCountIncoming() const;
    quint32 getPeerCountSum() const;

signals:
    void getTransfersSignal(const RpcApi::GetTransfers::Request& req);

public slots:
//    void addressesChanged(const QVariantMap& addresses);
//    void historyChanged(const QVariantMap& history);
//    void statusChanged(const QVariantMap& status);
//    void unspentsChanged(const QVariantMap& unspents);
//    void transactionSent(const QVariantMap& sendTransaction);
//    void errorOccured(const QString& message, const QString& description);

    void statusReceived(const RpcApi::Status& status);
    void transfersReceived(const RpcApi::Transfers& history);
    void addressesReceived(const RpcApi::Addresses& addresses);
    void balanceReceived(const RpcApi::Balance& balance);
    void viewKeyReceived(const RpcApi::ViewKey& viewKey);
    void unspentsReceived(const RpcApi::Unspents& unspents);

    void stateChanged(RemoteWalletd::State oldState, RemoteWalletd::State newState);

private:
    QVariant getEditRoleData(const QModelIndex& index) const;
    QVariant getDecorationRoleData(const QModelIndex& index) const;
    QVariant getToolTipRoleData(const QModelIndex& index) const;
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

//    virtual bool canFetchMore(const QModelIndex& parent) const override;
//    virtual void fetchMore(const QModelIndex& parent) override;

//    QPair<quint64, quint64> getUnspent() const;
//    qint64 getAmount(const QVariantMap& tx) const;

//    QVariantList addresses_;
//    QVariantList txs_;
//    QVariantMap status_;
//    QVariantMap unspents_;

    const int columnCount_;
    QScopedPointer<WalletModelState> pimpl_;

//    RpcApi::Status status_;
//    RpcApi::Balance balance_;
//    QList<RpcApi::Transaction> txs_;
////    QList<RpcApi::Transaction> confirmedTxs_;
////    QList<RpcApi::Transaction> unconfirmedTxs_;
//    QList<QString> addresses_;
////    RpcApi::ViewKey viewKey_;
////    RpcApi::Unspents unspents_;

//    RemoteWalletd::State state_;
////    bool canFetchMore_;
////    int unconfirmedTxsSize_;

//    const int columnCount_;
////    quint32 topReceivedBlock_;
////    quint32 bottomReceivedBlock_;
//    int unconfimedSize_;
//    bool canFetchMore_;
};

}

#endif // WALLETMODEL

