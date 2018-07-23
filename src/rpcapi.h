// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#ifndef RPCAPI_H
#define RPCAPI_H

#include <QString>
#include <QList>
#include <QMap>
#include <QMetaType>
#include <QVariantMap>
#include <QDateTime>

namespace RpcApi
{

typedef quint32 Height;
typedef quint64 Difficulty;
typedef quint64 Amount;
typedef quint64 Timestamp;
typedef qint64 SignedAmount;
typedef qint32 HeightOrDepth;
typedef quint64 UnlockMoment;  // Height or Timestamp,
typedef QString Hash;

constexpr HeightOrDepth DEFAULT_CONFIRMATIONS = 6;

struct EmptyStruct
{};

struct Output
{
    Amount amount = 0;
    QString public_key{};
    quint32 global_index = 0;

    UnlockMoment unlock_time = 0;
    quint32 index_in_transaction = 0;
    Height height = 0;
    QString key_image{};
    QString transaction_public_key{};
    QString address;
    bool dust = false;

    static Output fromJson(const QVariantMap& json);
    QVariantMap toJson() const;

    auto tie() const
    {
        return std::tie(
            amount,
            public_key,
            global_index,
            unlock_time,
            index_in_transaction,
            height,
            key_image,
            transaction_public_key,
            address,
            dust);
    }
};

struct Transfer
{
    QString address;
    SignedAmount amount = 0;
    bool ours = true;
    bool locked = false;
    QList<Output> outputs;

    static Transfer fromJson(const QVariantMap& json);
    QVariantMap toJson() const;

    auto tie() const
    {
        return std::tie(
            address,
            amount,
            ours,
            locked,
            outputs);
    }
};

struct Transaction
{
    UnlockMoment unlock_time = 0;
    QList<Transfer> transfers;
    Hash payment_id{};
    quint32 anonymity = 0;

    Hash hash{};
    SignedAmount fee = 0;
    QString public_key;
    QString extra;
    bool coinbase = false;
    Amount amount = 0;

    Height block_height = 0;
    Hash block_hash{};
    QDateTime timestamp;

    quint32 binary_size = 0;

    static Transaction fromJson(const QVariantMap& json);
    QVariantMap toJson() const;

    auto tie() const
    {
        return std::tie(
            unlock_time,
            transfers,
            payment_id,
            anonymity,
            hash,
            fee,
            public_key,
            extra,
            coinbase,
            amount,
            block_height,
            block_hash,
            timestamp,
            binary_size);
    }
};

struct BlockHeader
{
    quint8 major_version = 0;
    quint8 minor_version = 0;
    QDateTime timestamp;
    Hash previous_block_hash{};
    quint32 nonce = 0;

    Height height = 0;
    Hash hash{};
    Amount reward = 0;
    Difficulty cumulative_difficulty = 0;
    Difficulty difficulty = 0;
    Amount base_reward = 0;
    quint32 block_size = 0;
    quint32 transactions_cumulative_size = 0;
    Amount already_generated_coins = 0;
    quint64 already_generated_transactions = 0;
    quint32 size_median = 0;
    quint32 effective_size_median = 0;
    QDateTime timestamp_median;
    Amount total_fee_amount = 0;

    static BlockHeader fromJson(const QVariantMap& json);

    auto tie() const
    {
        return std::tie(
            major_version,
            minor_version,
            timestamp,
            previous_block_hash,
            nonce,
            height,
            hash,
            reward,
            cumulative_difficulty,
            difficulty,
            base_reward,
            block_size,
            transactions_cumulative_size,
            already_generated_coins,
            already_generated_transactions,
            size_median,
            effective_size_median,
            timestamp_median,
            total_fee_amount);
    }
};

struct Block
{
    BlockHeader header;
    QList<Transaction> transactions;

    static Block fromJson(const QVariantMap& json);

    auto tie() const
    {
        return std::tie(
            header,
            transactions);
    }
};

struct Proof
{
    QString message;
    QString address;
    Amount amount = 0;
    QString transaction_hash;
    QString proof;

    static Proof fromJson(const QVariantMap& json);

    auto tie() const
    {
        return std::tie(
            message,
            address,
            amount,
            transaction_hash,
            proof);
    }
};

struct GetStatus
{
    static constexpr char METHOD[] = "get_status";

    struct Request
    {
        Hash top_block_hash{};
        quint32 transaction_pool_version = 0;
        quint32 outgoing_peer_count = 0;
        quint32 incoming_peer_count = 0;
        QString lower_level_error;

        QVariantMap toJson() const;
    };

    struct Response
    {
        Hash top_block_hash = 0;
        quint32 transaction_pool_version = 0;
        quint32 outgoing_peer_count = 0;
        quint32 incoming_peer_count = 0;
        QString lower_level_error{"Disconnected"};

        Height top_block_height = 0;
        Height top_known_block_height = 0;
        Difficulty top_block_difficulty = 0;
        Difficulty top_block_cumulative_difficulty = 0;
        Amount recommended_fee_per_byte = 0;
        QDateTime top_block_timestamp;
        QDateTime top_block_timestamp_median;
        quint32 next_block_effective_median_size = 0;

        static Response fromJson(const QVariantMap& json);

        auto tie() const
        {
            return std::tie(
                top_block_hash,
                transaction_pool_version,
                outgoing_peer_count,
                incoming_peer_count,
                lower_level_error,
                top_block_height,
                top_known_block_height,
                top_block_difficulty,
                top_block_cumulative_difficulty,
                recommended_fee_per_byte,
                top_block_timestamp,
                top_block_timestamp_median,
                next_block_effective_median_size);
        }
    };
};

struct GetAddresses
{
    static constexpr char METHOD[] = "get_addresses";

    using Request = EmptyStruct;

    struct Response
    {
        QStringList addresses;
        bool view_only = false; // TODO show flag in gui

        static Response fromJson(const QVariantMap& json);
    };
};

struct GetViewKey
{
    static constexpr char METHOD[] = "get_view_key_pair";

    using Request = EmptyStruct;

    struct Response
    {
        QString secret_view_key;
        QString public_view_key;

        static Response fromJson(const QVariantMap& json);
    };
};

struct GetBalance
{
    static constexpr char METHOD[] = "get_balance";

    struct Request
    {
        QString address{};
        HeightOrDepth height_or_depth = -DEFAULT_CONFIRMATIONS - 1;

        QVariantMap toJson() const;
    };

    struct Response
    {
        Amount spendable = 0;
        Amount spendable_dust = 0;
        Amount locked_or_unconfirmed = 0;
        quint64 spendable_outputs = 0;
        quint64 spendable_dust_outputs = 0;
        quint64 locked_or_unconfirmed_outputs = 0;

        static Response fromJson(const QVariantMap& json);

        auto tie() const
        {
            return std::tie(
                spendable,
                spendable_dust,
                locked_or_unconfirmed,
                spendable_outputs,
                spendable_dust_outputs,
                locked_or_unconfirmed_outputs);
        }
    };
};

struct GetUnspents
{
    static constexpr char METHOD[] = "get_unspents";

    struct Request
    {
        QString address{};
        HeightOrDepth height_or_depth = -DEFAULT_CONFIRMATIONS - 1;

//        QVariantMap toJson() const;
    };

    struct Response
    {
        QList<Output> unspents;
        QList<Output> unspendable_unspents;

//        static Response fromJson(const QVariantMap& json);
    };
};

struct GetTransfers
{
    static constexpr char METHOD[] = "get_transfers";

    struct Request
    {
        QString address;
        Height from_height = 0;
        Height to_height = std::numeric_limits<Height>::max();
        bool forward = false;
        uint32_t desired_transactions_count = 50;

        QVariantMap toJson() const;
    };

    struct Response
    {
        QList<Block> blocks;
        QList<Transfer> unlocked_transfers;
        Height next_from_height = 0;
        Height next_to_height = 0;

        static Response fromJson(const QVariantMap& json);
    };
};

struct CreateTransaction
{
    static constexpr char METHOD[] = "create_transaction";

    struct Request
    {
        Transaction transaction;
        QStringList spend_addresses;
        bool any_spend_address = false;
        QString change_address;
        HeightOrDepth confirmed_height_or_depth = -DEFAULT_CONFIRMATIONS - 1;
        SignedAmount fee_per_byte = 0;
        QString optimization;
        bool save_history = true;
        QList<Hash> prevent_conflict_with_transactions;

        QVariantMap toJson() const;
    };

    struct Response
    {
        QString binary_transaction;
        Transaction transaction;
        bool save_history_error = false;
        QList<Hash> transactions_required;

        static Response fromJson(const QVariantMap& json);
    };
};

struct SendTransaction
{
    static constexpr char METHOD[] = "send_transaction";

    struct Request
    {
        QString binary_transaction;

        QVariantMap toJson() const;
    };

    struct Response
    {
        QString send_result;

        static Response fromJson(const QVariantMap& json);
    };
};

struct CreateSendProof
{
    static constexpr char METHOD[] = "create_send_proof";

    struct Request
    {
        Hash transaction_hash;
        QString message;
        QStringList addresses;

        QVariantMap toJson() const;
    };

    struct Response
    {
        QStringList send_proofs;

        static Response fromJson(const QVariantMap& json);
    };
};

struct CheckSendProof
{
    static constexpr char METHOD[] = "check_send_proof";

    struct Request
    {
        QString send_proof;

        QVariantMap toJson() const;
    };

    struct Response
    {
        QString validation_error;

        static Response fromJson(const QVariantMap& json);
    };
};

using Status = GetStatus::Response;
using Transfers = GetTransfers::Response;
using Addresses = GetAddresses::Response;
using Balance = GetBalance::Response;
using ViewKey = GetViewKey::Response;
using Unspents = GetUnspents::Response;
using CreatedTx = CreateTransaction::Response;
using SentTx = SendTransaction::Response;
using Proofs = CreateSendProof::Response;
using ProofCheck = CheckSendProof::Response;

inline bool operator == (const Status& lhs, const Status& rhs)
{ return lhs.tie() == rhs.tie(); }

inline bool operator == (const Balance& lhs, const Balance& rhs)
{ return lhs.tie() == rhs.tie(); }

inline bool operator == (const BlockHeader& lhs, const BlockHeader& rhs)
{ return lhs.tie() == rhs.tie(); }

inline bool operator == (const Block& lhs, const Block& rhs)
{ return lhs.tie() == rhs.tie(); }

inline bool operator == (const Transaction& lhs, const Transaction& rhs)
{ return lhs.tie() == rhs.tie(); }

inline bool operator == (const Transfer& lhs, const Transfer& rhs)
{ return lhs.tie() == rhs.tie(); }

inline bool operator == (const Output& lhs, const Output& rhs)
{ return lhs.tie() == rhs.tie(); }

}

#endif // RPCAPI_H

