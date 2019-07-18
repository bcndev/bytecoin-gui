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
#include "common.h"

namespace RpcApi
{

typedef quint32 Height;
typedef quint64 Difficulty;
typedef quint64 Amount;
typedef quint64 Timestamp;
typedef qint64 SignedAmount;
typedef qint32 HeightOrDepth;
typedef quint64 BlockOrTimestamp;  // Height or Timestamp,
typedef QString Hash;

constexpr HeightOrDepth DEFAULT_CONFIRMATIONS = CONFIRMATIONS;
constexpr char MAIN_NET_NAME[] = "main";
constexpr char STAGE_NET_NAME[] = "stage";
constexpr char TEST_NET_NAME[] = "test";


enum class ErrorCodes
{
    INVALID_HEIGHT_OR_DEPTH           = -2,  // height_or_depth too low or too high
    TOO_BIG_RESERVE_SIZE              = -3,
    ADDRESS_FAILED_TO_PARSE           = -4,  // returns ErrorAddress
    HASH_NOT_FOUND                    = -5,  // Neither in main nor in side chain
    WRONG_BLOCKBLOB                   = -6,
    BLOCK_NOT_ACCEPTED                = -7,
    INVALID_TRANSACTION_BINARY_FORMAT = -101,
    WRONG_OUTPUT_REFERENCE            = -102,  // wrong signature or referenced outputs changed during reorg. Bad output
    // height is reported in conflict_height. If output index > max current index, conflict_height will be set to
    // currency.max_block_number
    OUTPUT_ALREADY_SPENT              = -103, // conflight height reported in error
    FAILED_TO_PARSE                   = -201,
    NOT_IN_MAIN_CHAIN                 = -202,
    WRONG_SIGNATURE                   = -203,
    ADDRESS_NOT_IN_TRANSACTION        = -204,
    WRONG_AMOUNT                      = -205,
    NOT_ENOUGH_FUNDS                  = -301,
    TRANSACTION_DOES_NOT_FIT_IN_BLOCK = -302,
    NOT_ENOUGH_ANONYMITY              = -303,
    VIEW_ONLY_WALLET                  = -304,
    TOO_MUCH_ANONYMITY                = -305, // For white addresses, anonymity should be 0 when sending
    WRONG_ARCHIVE_ID                  = -501, // If archive id changed, it is returned in Error
    ARCHIVE_NOT_ENABLED               = -502, // No archive on this node
    ADDRESS_NOT_IN_WALLET             = -1002, // returns ErrorAddress
};


struct EmptyStruct
{
    static EmptyStruct fromJson(const QVariantMap&)
    { return EmptyStruct{}; }
};

struct Output
{
    Amount amount = 0;
    QString public_key{};
    quint64 stack_index = 0;
    quint64 global_index = 0;

    Height height = 0;
    BlockOrTimestamp unlock_block_or_timestamp = 0;
    quint64 index_in_transaction = 0;

    QString transaction_hash;
    QString key_image;
    QString address;
    bool dust = false;

    static Output fromJson(const QVariantMap& json);
    QVariantMap toJson() const;

    auto tie() const
    {
        return std::tie(
            amount,
            public_key,
            stack_index,
            global_index,
            height,
            unlock_block_or_timestamp,
            index_in_transaction,
            transaction_hash,
            key_image,
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
    // we do not request outputs
//    QList<Output> outputs;
    QString transaction_hash;

    static Transfer fromJson(const QVariantMap& json);
    QVariantMap toJson() const;

    auto tie() const
    {
        return std::tie(
            address,
            amount,
            ours,
            locked,
//            outputs,
            transaction_hash);
    }
};

struct Transaction
{
    BlockOrTimestamp unlock_block_or_timestamp = 0;
    QList<Transfer> transfers;
    Hash payment_id{};
    quint64 anonymity = 0;

    Hash hash;
    Hash prefix_hash;
    Hash inputs_hash;
    Amount fee = 0;
    QString public_key;
    QString extra;
    bool coinbase = false;
    Amount amount = 0;

    Height block_height = 0;
    Hash block_hash;
    QDateTime timestamp;

    quint64 size = 0;

    static Transaction fromJson(const QVariantMap& json);
    QVariantMap toJson() const;

    auto tie() const
    {
        return std::tie(
            unlock_block_or_timestamp,
            transfers,
            payment_id,
            anonymity,
            hash,
            prefix_hash,
            inputs_hash,
            fee,
            public_key,
            extra,
            coinbase,
            amount,
            block_height,
            block_hash,
            timestamp,
            size);
    }
};

struct BlockHeader
{
    quint8 major_version = 0;
    quint8 minor_version = 0;
    QDateTime timestamp;
    Hash previous_block_hash{};
    QString binary_nonce;

    Height height = 0;
    Hash hash;
    Amount reward = 0;
    quint64 cumulative_difficulty = 0;
    quint64 cumulative_difficulty_hi = 0;
    Difficulty difficulty = 0;
    Amount base_reward = 0;
    quint64 block_size = 0;
    quint64 transactions_size = 0;
    Amount already_generated_coins = 0;
    quint64 already_generated_transactions = 0;
    quint64 already_generated_key_outputs = 0;
    quint64 size_median = 0;
    quint64 effective_size_median = 0;
    quint64 block_capacity_vote = 0;
    quint64 block_capacity_vote_median = 0;
    QDateTime timestamp_median;
    Amount transactions_fee = 0;

    static BlockHeader fromJson(const QVariantMap& json);

    auto tie() const
    {
        return std::tie(
            major_version,
            minor_version,
            timestamp,
            previous_block_hash,
            binary_nonce,
            height,
            hash,
            reward,
            cumulative_difficulty,
            cumulative_difficulty_hi,
            difficulty,
            base_reward,
            block_size,
            transactions_size,
            already_generated_coins,
            already_generated_transactions,
            already_generated_key_outputs,
            size_median,
            effective_size_median,
            block_capacity_vote,
            block_capacity_vote_median,
            timestamp_median,
            transactions_fee);
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

struct WalletRecord
{
    QString address;
    QString label;
    quint64 index = 0;
    // we do not ask keys
//    QString secret_spend_key;
//    QString public_spend_key;

    static WalletRecord fromJson(const QVariantMap& json);

    auto tie() const
    {
        return std::tie(
            address,
            label,
            index/*,
            secret_spend_key,
            public_spend_key*/);
    }
};

struct GetStatus
{
    static constexpr char METHOD[] = "get_status";

    struct Request
    {
        Hash top_block_hash{};
        quint64 transaction_pool_version = 0;
        quint64 outgoing_peer_count = 0;
        quint64 incoming_peer_count = 0;
        QString lower_level_error;

        QVariantMap toJson() const;
    };

    struct Response
    {
        Hash top_block_hash = 0;
        quint64 transaction_pool_version = 0;
        quint64 outgoing_peer_count = 0;
        quint64 incoming_peer_count = 0;
        QString lower_level_error{"Disconnected"};

        Height top_block_height = 0;
        Height top_known_block_height = 0;
        Difficulty top_block_difficulty = 0;
        quint64 top_block_cumulative_difficulty = 0;
        quint64 top_block_cumulative_difficulty_hi = 0;
        Amount recommended_fee_per_byte = 0;
        QDateTime top_block_timestamp;
        QDateTime top_block_timestamp_median;
        quint64 recommended_max_transaction_size = 0;

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
                top_block_cumulative_difficulty_hi,
                recommended_fee_per_byte,
                top_block_timestamp,
                top_block_timestamp_median,
                recommended_max_transaction_size);
        }
    };
};

struct GetAddresses
{
    static constexpr char METHOD[] = "get_addresses";

    struct Request
    {
        bool need_secret_spend_keys = false;
        quint64 from_address = 0; // We can now iterate through addresses
        quint64 max_count = 1; // we need only 1 address in gui

        QVariantMap toJson() const;
    };

    struct Response
    {
        QStringList addresses;
        QStringList secret_spend_keys;
        quint64 total_address_count = 0;

        static Response fromJson(const QVariantMap& json);
    };
};

struct GetWalletInfo
{
    static constexpr char METHOD[] = "get_wallet_info";

    struct Request
    {
        bool need_secrets = false;

        QVariantMap toJson() const;
    };


    struct Response
    {
        bool view_only = false;
        QString wallet_type;
        bool can_view_outgoing_addresses = false;
        bool has_view_secret_key = false;
        QDateTime wallet_creation_timestamp;
        QString first_address;
        quint64 total_address_count = 0;
        QString net = MAIN_NET_NAME;
        QString secret_view_key;
        QString public_view_key;
        QString import_keys; // for old wallets
        QString mnemonic;    // for HD wallets

        static Response fromJson(const QVariantMap& json);

        auto tie() const
        {
            return std::tie(
                view_only,
                wallet_type,
                can_view_outgoing_addresses,
                has_view_secret_key,
                wallet_creation_timestamp,
                first_address,
                total_address_count,
                net,
                secret_view_key,
                public_view_key,
                import_keys,
                mnemonic);
        }
    };
};

struct GetWalletRecords
{
    static constexpr char METHOD[] = "get_wallet_records";

    struct Request
    {
        bool need_secrets = false;
        bool create = false;
        quint32 index = 0;
        quint32 count  = std::numeric_limits<quint32>::max();

        QVariantMap toJson() const;
    };

    struct Response
    {
        QList<WalletRecord> records;
        quint64 total_count = 0;

        static Response fromJson(const QVariantMap& json);
    };
};

struct SetAddressLabel
{
    static constexpr char METHOD[] = "set_address_label";

    struct Request
    {
        QString address;
        QString label;

        QVariantMap toJson() const;
    };

    using Response = EmptyStruct;
};

struct CreateAddresses
{
    static constexpr char METHOD[] = "create_addresses";

    struct Request
    {
        QStringList secret_spend_keys;
        QDateTime creation_timestamp;

        QVariantMap toJson() const;
    };

    struct Response
    {
        QStringList addresses;
        QStringList secret_spend_keys;

        static Response fromJson(const QVariantMap& json);
    };
};

struct GetBalance
{
    static constexpr char METHOD[] = "get_balance";

    struct Request
    {
        QString address;
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
        QList<Output> spendable;
        QList<Output> locked_or_unconfirmed;

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
        quint64 desired_transactions_count = 300;
        bool need_outputs = false;

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
        bool subtract_fee_from_amount = false;
        QStringList prevent_conflict_with_transactions;

        QVariantMap toJson() const;
    };

    struct Response
    {
        QString binary_transaction;
        Transaction transaction;
        bool save_history_error = false;
        QStringList transactions_required;

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

struct GetTransaction
{
    static constexpr char METHOD[] = "get_transaction";

    struct Request
    {
        Hash hash;

        QVariantMap toJson() const;
    };

    struct Response
    {
        Transaction transaction;

        static Response fromJson(const QVariantMap& json);
    };
};

struct CreateSendProof
{
    static constexpr char METHOD[] = "create_sendproof";

    struct Request
    {
        Hash transaction_hash;
        QString message;
        QStringList addresses;

        QVariantMap toJson() const;
    };

    struct Response
    {
        QStringList sendproofs;

        static Response fromJson(const QVariantMap& json);
    };
};

struct CheckSendProof
{
    static constexpr char METHOD[] = "check_sendproof";

    struct Request
    {
        QString sendproof;

        QVariantMap toJson() const;
    };

    struct Response
    {
        QString validation_error; // do not remove

        Hash transaction_hash;
        QString address;
        Amount amount = 0;
        QString message;
        QList<quint64> output_indexes;

        static Response fromJson(const QVariantMap& json);
    };
};

using Status = GetStatus::Response;
using Transfers = GetTransfers::Response;
using WalletInfo = GetWalletInfo::Response;
using Balance = GetBalance::Response;
using Unspents = GetUnspents::Response;
using CreatedTx = CreateTransaction::Response;
using SentTx = SendTransaction::Response;
using Proofs = CreateSendProof::Response;
using ProofCheck = CheckSendProof::Response;
using WalletRecords = GetWalletRecords::Response;
using CreatedAddresses = CreateAddresses::Response;

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

