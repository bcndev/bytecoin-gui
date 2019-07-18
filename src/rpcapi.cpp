// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QVariantMap>

#include "rpcapi.h"

namespace RpcApi
{


constexpr char GetStatus::METHOD[];
constexpr char GetAddresses::METHOD[];
constexpr char GetWalletInfo::METHOD[];
constexpr char GetTransfers::METHOD[];
constexpr char GetBalance::METHOD[];
constexpr char GetUnspents::METHOD[];
constexpr char CreateTransaction::METHOD[];
constexpr char SendTransaction::METHOD[];
constexpr char CreateSendProof::METHOD[];
constexpr char CheckSendProof::METHOD[];
constexpr char GetWalletRecords::METHOD[];
constexpr char SetAddressLabel::METHOD[];
constexpr char CreateAddresses::METHOD[];

#define RPCAPI_SERIALIZE_FIELD(obj, json, fieldName) \
    do \
    { \
        json.insert(#fieldName, obj.fieldName); \
    } \
    while (0)

#define RPCAPI_SERIALIZE_TIMESTAMP(obj, json, fieldName) \
    do \
    { \
        json.insert(#fieldName, obj.fieldName.toTime_t()); \
    } \
    while (0)

#define RPCAPI_SERIALIZE_STRUCT(obj, json, fieldName) \
    do \
    { \
        json.insert(#fieldName, obj.fieldName.toJson()); \
    } \
    while (0)

#define RPCAPI_SERIALIZE_LIST(obj, json, fieldName) \
    do \
    { \
        using FieldType = decltype(obj.fieldName); \
        QVariantList vlist; \
        for (const FieldType::value_type& var: obj.fieldName) \
            vlist << var.toJson(); \
        json.insert(#fieldName, vlist); \
    } \
    while (0)

#define RPCAPI_DESERIALIZE_FIELD(obj, json, fieldName) \
    do \
    { \
        using FieldType = decltype(obj.fieldName); \
        if (!json.contains(#fieldName)) \
            qDebug("[RpcApi] Field '%s' not found. Using default.", #fieldName); \
        else if (!json[#fieldName].canConvert<FieldType>()) \
            qDebug("[RpcApi] Cannot convert '%s'.", #fieldName); \
        else \
            obj.fieldName = json[#fieldName].value<FieldType>(); \
    } \
    while (0)

#define RPCAPI_DESERIALIZE_TIMESTAMP(obj, json, fieldName) \
    do \
    { \
        using FieldType = quint64; \
        if (!json.contains(#fieldName)) \
            qDebug("[RpcApi] Field '%s' not found. Using default.", #fieldName); \
        else if (!json[#fieldName].canConvert<FieldType>()) \
            qDebug("[RpcApi] Cannot convert '%s'.", #fieldName); \
        else \
            obj.fieldName = QDateTime::fromTime_t(json[#fieldName].value<FieldType>()).toUTC(); \
    } \
    while (0)

#define RPCAPI_DESERIALIZE_LIST(obj, json, fieldName) \
    do \
    { \
        using FieldType = decltype(obj.fieldName); \
        if (!json.contains(#fieldName)) \
            qDebug("[RpcApi] Field '%s' not found. Using default.", #fieldName); \
        else if (!json[#fieldName].canConvert<QVariantList>()) \
            qDebug("[RpcApi] Cannot convert '%s'.", #fieldName); \
        else \
        { \
            QVariantList vlist = json[#fieldName].toList(); \
            for (const QVariant& var : vlist) \
                obj.fieldName << FieldType::value_type::fromJson(var.toMap()); \
        } \
    } \
    while (0)

#define RPCAPI_DESERIALIZE_STRUCT(obj, json, fieldName) \
    do \
    { \
        using FieldType = decltype(obj.fieldName); \
        if (!json.contains(#fieldName)) \
            qDebug("[RpcApi] Field '%s' not found. Using default.", #fieldName); \
        else \
        { \
            obj.fieldName = FieldType::fromJson(json[#fieldName].toMap()); \
        } \
    } \
    while (0)

/*static*/
GetStatus::Response
GetStatus::Response::fromJson(const QVariantMap& json)
{
    GetStatus::Response value;

    RPCAPI_DESERIALIZE_FIELD(value, json, top_block_hash);
    RPCAPI_DESERIALIZE_FIELD(value, json, transaction_pool_version);
    RPCAPI_DESERIALIZE_FIELD(value, json, outgoing_peer_count);
    RPCAPI_DESERIALIZE_FIELD(value, json, incoming_peer_count);
    RPCAPI_DESERIALIZE_FIELD(value, json, lower_level_error);
    RPCAPI_DESERIALIZE_FIELD(value, json, top_block_height);
    RPCAPI_DESERIALIZE_FIELD(value, json, top_block_difficulty);
    RPCAPI_DESERIALIZE_FIELD(value, json, top_block_cumulative_difficulty);
//    RPCAPI_DESERIALIZE_FIELD(value, json, top_block_cumulative_difficulty_hi);

    RPCAPI_DESERIALIZE_TIMESTAMP(value, json, top_block_timestamp);
    RPCAPI_DESERIALIZE_TIMESTAMP(value, json, top_block_timestamp_median);

    RPCAPI_DESERIALIZE_FIELD(value, json, recommended_max_transaction_size);
    RPCAPI_DESERIALIZE_FIELD(value, json, recommended_fee_per_byte);
    RPCAPI_DESERIALIZE_FIELD(value, json, top_known_block_height);

    return value;
}

QVariantMap
GetStatus::Request::toJson() const
{
    const GetStatus::Request& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, top_block_hash);
    RPCAPI_SERIALIZE_FIELD(value, json, transaction_pool_version);
    RPCAPI_SERIALIZE_FIELD(value, json, outgoing_peer_count);
    RPCAPI_SERIALIZE_FIELD(value, json, incoming_peer_count);
    RPCAPI_SERIALIZE_FIELD(value, json, lower_level_error);

    return json;
}

/*static*/
GetAddresses::Response
GetAddresses::Response::fromJson(const QVariantMap& json)
{
    GetAddresses::Response value;

    RPCAPI_DESERIALIZE_FIELD(value, json, addresses);
    RPCAPI_DESERIALIZE_FIELD(value, json, secret_spend_keys);
    RPCAPI_DESERIALIZE_FIELD(value, json, total_address_count);

    return value;
}

QVariantMap
GetAddresses::Request::toJson() const
{
    const GetAddresses::Request& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, need_secret_spend_keys);
    RPCAPI_SERIALIZE_FIELD(value, json, from_address);
    RPCAPI_SERIALIZE_FIELD(value, json, max_count);

    return json;
}

/*static*/
GetWalletInfo::Response
GetWalletInfo::Response::fromJson(const QVariantMap& json)
{
    GetWalletInfo::Response value;

    RPCAPI_DESERIALIZE_FIELD(value, json, view_only);
    RPCAPI_DESERIALIZE_FIELD(value, json, wallet_type);
    RPCAPI_DESERIALIZE_FIELD(value, json, can_view_outgoing_addresses);
    RPCAPI_DESERIALIZE_FIELD(value, json, has_view_secret_key);
    RPCAPI_DESERIALIZE_TIMESTAMP(value, json, wallet_creation_timestamp);
    RPCAPI_DESERIALIZE_FIELD(value, json, first_address);
    RPCAPI_DESERIALIZE_FIELD(value, json, total_address_count);
    RPCAPI_DESERIALIZE_FIELD(value, json, net);
//    RPCAPI_DESERIALIZE_FIELD(value, json, secret_view_key);
//    RPCAPI_DESERIALIZE_FIELD(value, json, public_view_key);
//    RPCAPI_DESERIALIZE_FIELD(value, json, import_keys);
//    RPCAPI_DESERIALIZE_FIELD(value, json, mnemonic);

    return value;
}

QVariantMap
GetWalletInfo::Request::toJson() const
{
    const GetWalletInfo::Request& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, need_secrets);

    return json;
}

/*static*/
GetBalance::Response
GetBalance::Response::fromJson(const QVariantMap& json)
{
    GetBalance::Response value;

    RPCAPI_DESERIALIZE_FIELD(value, json, spendable);
    RPCAPI_DESERIALIZE_FIELD(value, json, spendable_dust);
    RPCAPI_DESERIALIZE_FIELD(value, json, locked_or_unconfirmed);
    RPCAPI_DESERIALIZE_FIELD(value, json, spendable_outputs);
    RPCAPI_DESERIALIZE_FIELD(value, json, spendable_dust_outputs);
    RPCAPI_DESERIALIZE_FIELD(value, json, locked_or_unconfirmed_outputs);

    return value;
}

QVariantMap
GetBalance::Request::toJson() const
{
    const GetBalance::Request& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, address);
    RPCAPI_SERIALIZE_FIELD(value, json, height_or_depth);

    return json;
}

/*static*/
GetTransfers::Response
GetTransfers::Response::fromJson(const QVariantMap& json)
{
    GetTransfers::Response value;

    RPCAPI_DESERIALIZE_LIST(value, json, blocks);
    RPCAPI_DESERIALIZE_LIST(value, json, unlocked_transfers);
    RPCAPI_DESERIALIZE_FIELD(value, json, next_from_height);
    RPCAPI_DESERIALIZE_FIELD(value, json, next_to_height);

    return value;
}

QVariantMap
GetTransfers::Request::toJson() const
{
    const GetTransfers::Request& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, address);
    RPCAPI_SERIALIZE_FIELD(value, json, from_height);
    RPCAPI_SERIALIZE_FIELD(value, json, to_height);
    RPCAPI_SERIALIZE_FIELD(value, json, forward);
    RPCAPI_SERIALIZE_FIELD(value, json, desired_transactions_count);
    RPCAPI_SERIALIZE_FIELD(value, json, need_outputs);

    return json;
}

/*static*/
CreateTransaction::Response
CreateTransaction::Response::fromJson(const QVariantMap& json)
{
    CreateTransaction::Response value;

    RPCAPI_DESERIALIZE_FIELD(value, json, binary_transaction);
    RPCAPI_DESERIALIZE_FIELD(value, json, save_history_error);
    RPCAPI_DESERIALIZE_FIELD(value, json, transactions_required);

    RPCAPI_DESERIALIZE_STRUCT(value, json, transaction);

    return value;
}

QVariantMap
CreateTransaction::Request::toJson() const
{
    const CreateTransaction::Request& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_STRUCT(value, json, transaction);

    RPCAPI_SERIALIZE_FIELD(value, json, spend_addresses);
    RPCAPI_SERIALIZE_FIELD(value, json, any_spend_address);
    RPCAPI_SERIALIZE_FIELD(value, json, change_address);
    RPCAPI_SERIALIZE_FIELD(value, json, confirmed_height_or_depth);
    RPCAPI_SERIALIZE_FIELD(value, json, fee_per_byte);
    RPCAPI_SERIALIZE_FIELD(value, json, optimization);
    RPCAPI_SERIALIZE_FIELD(value, json, save_history);
    RPCAPI_SERIALIZE_FIELD(value, json, subtract_fee_from_amount);
    RPCAPI_SERIALIZE_FIELD(value, json, prevent_conflict_with_transactions);

    return json;
}

/*static*/
SendTransaction::Response
SendTransaction::Response::fromJson(const QVariantMap& json)
{
    SendTransaction::Response value;

    RPCAPI_DESERIALIZE_FIELD(value, json, send_result);

    return value;
}

QVariantMap
SendTransaction::Request::toJson() const
{
    const SendTransaction::Request& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, binary_transaction);

    return json;
}

/*static*/
Output
Output::fromJson(const QVariantMap& json)
{
    Output value;

    RPCAPI_DESERIALIZE_FIELD(value, json, amount);
    RPCAPI_DESERIALIZE_FIELD(value, json, public_key);
    RPCAPI_DESERIALIZE_FIELD(value, json, stack_index);
    RPCAPI_DESERIALIZE_FIELD(value, json, global_index);
    RPCAPI_DESERIALIZE_FIELD(value, json, height);
    RPCAPI_DESERIALIZE_FIELD(value, json, unlock_block_or_timestamp);
    RPCAPI_DESERIALIZE_FIELD(value, json, index_in_transaction);
    RPCAPI_DESERIALIZE_FIELD(value, json, transaction_hash);
    RPCAPI_DESERIALIZE_FIELD(value, json, key_image);
    RPCAPI_DESERIALIZE_FIELD(value, json, address);
    RPCAPI_DESERIALIZE_FIELD(value, json, dust);

    return value;
}

QVariantMap
Output::toJson() const
{
    const Output& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, amount);
    RPCAPI_SERIALIZE_FIELD(value, json, public_key);
    RPCAPI_SERIALIZE_FIELD(value, json, stack_index);
    RPCAPI_SERIALIZE_FIELD(value, json, global_index);
    RPCAPI_SERIALIZE_FIELD(value, json, height);
    RPCAPI_SERIALIZE_FIELD(value, json, unlock_block_or_timestamp);
    RPCAPI_SERIALIZE_FIELD(value, json, index_in_transaction);
    RPCAPI_SERIALIZE_FIELD(value, json, transaction_hash);
    RPCAPI_SERIALIZE_FIELD(value, json, key_image);
    RPCAPI_SERIALIZE_FIELD(value, json, address);
    RPCAPI_SERIALIZE_FIELD(value, json, dust);

    return json;
}

/*static*/
Transfer
Transfer::fromJson(const QVariantMap& json)
{
    Transfer value;

    RPCAPI_DESERIALIZE_FIELD(value, json, address);
    RPCAPI_DESERIALIZE_FIELD(value, json, amount);
    RPCAPI_DESERIALIZE_FIELD(value, json, ours);
    RPCAPI_DESERIALIZE_FIELD(value, json, locked);
    RPCAPI_DESERIALIZE_FIELD(value, json, transaction_hash);

//    RPCAPI_DESERIALIZE_LIST(value, json, outputs);

    return value;
}

QVariantMap
Transfer::toJson() const
{
    const Transfer& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, address);
    RPCAPI_SERIALIZE_FIELD(value, json, amount);
    RPCAPI_SERIALIZE_FIELD(value, json, ours);
    RPCAPI_SERIALIZE_FIELD(value, json, locked);
    RPCAPI_SERIALIZE_FIELD(value, json, transaction_hash);

//    RPCAPI_SERIALIZE_LIST(value, json, outputs);

    return json;
}

/*static*/
BlockHeader
BlockHeader::fromJson(const QVariantMap& json)
{
    BlockHeader value;

    RPCAPI_DESERIALIZE_FIELD(value, json, major_version);
    RPCAPI_DESERIALIZE_FIELD(value, json, minor_version);
    RPCAPI_DESERIALIZE_TIMESTAMP(value, json, timestamp);
    RPCAPI_DESERIALIZE_FIELD(value, json, previous_block_hash);
    RPCAPI_DESERIALIZE_FIELD(value, json, binary_nonce);

    RPCAPI_DESERIALIZE_FIELD(value, json, height);
    RPCAPI_DESERIALIZE_FIELD(value, json, hash);
    RPCAPI_DESERIALIZE_FIELD(value, json, reward);
    RPCAPI_DESERIALIZE_FIELD(value, json, cumulative_difficulty);
//    RPCAPI_DESERIALIZE_FIELD(value, json, cumulative_difficulty_hi);
    RPCAPI_DESERIALIZE_FIELD(value, json, difficulty);
    RPCAPI_DESERIALIZE_FIELD(value, json, base_reward);
    RPCAPI_DESERIALIZE_FIELD(value, json, block_size);
    RPCAPI_DESERIALIZE_FIELD(value, json, transactions_size);
    RPCAPI_DESERIALIZE_FIELD(value, json, already_generated_coins);
    RPCAPI_DESERIALIZE_FIELD(value, json, already_generated_transactions);
    RPCAPI_DESERIALIZE_FIELD(value, json, already_generated_key_outputs);
    RPCAPI_DESERIALIZE_FIELD(value, json, size_median);
    RPCAPI_DESERIALIZE_FIELD(value, json, effective_size_median);
    RPCAPI_DESERIALIZE_FIELD(value, json, block_capacity_vote);
    RPCAPI_DESERIALIZE_FIELD(value, json, block_capacity_vote_median);
    RPCAPI_DESERIALIZE_TIMESTAMP(value, json, timestamp_median);
    RPCAPI_DESERIALIZE_FIELD(value, json, transactions_fee);

    return value;
}

/*static*/
Block
Block::fromJson(const QVariantMap& json)
{
    Block value;

    RPCAPI_DESERIALIZE_STRUCT(value, json, header);
    RPCAPI_DESERIALIZE_LIST(value, json, transactions);

    return value;
}

/*static*/
WalletRecord
WalletRecord::fromJson(const QVariantMap& json)
{
    WalletRecord value;

    RPCAPI_DESERIALIZE_FIELD(value, json, address);
    RPCAPI_DESERIALIZE_FIELD(value, json, label);
    RPCAPI_DESERIALIZE_FIELD(value, json, index);
//    RPCAPI_DESERIALIZE_FIELD(value, json, secret_spend_key);
//    RPCAPI_DESERIALIZE_FIELD(value, json, public_spend_key);

    return value;
}

/*static*/
Transaction
Transaction::fromJson(const QVariantMap& json)
{
    Transaction value;

    RPCAPI_DESERIALIZE_FIELD(value, json, unlock_block_or_timestamp);
//    RPCAPI_DESERIALIZE_FIELD(value, json, payment_id);
    RPCAPI_DESERIALIZE_FIELD(value, json, anonymity);
    RPCAPI_DESERIALIZE_FIELD(value, json, hash);
    RPCAPI_DESERIALIZE_FIELD(value, json, prefix_hash);
    RPCAPI_DESERIALIZE_FIELD(value, json, inputs_hash);
    RPCAPI_DESERIALIZE_FIELD(value, json, fee);
    RPCAPI_DESERIALIZE_FIELD(value, json, public_key);
    RPCAPI_DESERIALIZE_FIELD(value, json, extra);
    RPCAPI_DESERIALIZE_FIELD(value, json, coinbase);
    RPCAPI_DESERIALIZE_FIELD(value, json, amount);
    RPCAPI_DESERIALIZE_FIELD(value, json, block_height);
    RPCAPI_DESERIALIZE_FIELD(value, json, block_hash);
    RPCAPI_DESERIALIZE_FIELD(value, json, size);

    RPCAPI_DESERIALIZE_LIST(value, json, transfers);
    RPCAPI_DESERIALIZE_TIMESTAMP(value, json, timestamp);

    return value;
}

QVariantMap
Transaction::toJson() const
{
    const Transaction& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, unlock_block_or_timestamp);
    RPCAPI_SERIALIZE_FIELD(value, json, payment_id);
    RPCAPI_SERIALIZE_FIELD(value, json, anonymity);
    RPCAPI_SERIALIZE_FIELD(value, json, hash);
    RPCAPI_SERIALIZE_FIELD(value, json, prefix_hash);
    RPCAPI_SERIALIZE_FIELD(value, json, inputs_hash);
    RPCAPI_SERIALIZE_FIELD(value, json, fee);
    RPCAPI_SERIALIZE_FIELD(value, json, public_key);
    RPCAPI_SERIALIZE_FIELD(value, json, extra);
    RPCAPI_SERIALIZE_FIELD(value, json, coinbase);
    RPCAPI_SERIALIZE_FIELD(value, json, amount);
    RPCAPI_SERIALIZE_FIELD(value, json, block_height);
    RPCAPI_SERIALIZE_FIELD(value, json, block_hash);
    RPCAPI_SERIALIZE_FIELD(value, json, size);

    RPCAPI_SERIALIZE_LIST(value, json, transfers);
    RPCAPI_SERIALIZE_TIMESTAMP(value, json, timestamp);

    return json;
}

/*static*/
//Proof
//Proof::fromJson(const QVariantMap& json)
//{
//    Proof value;

//    RPCAPI_DESERIALIZE_FIELD(value, json, message);
//    RPCAPI_DESERIALIZE_FIELD(value, json, address);
//    RPCAPI_DESERIALIZE_FIELD(value, json, amount);
//    RPCAPI_DESERIALIZE_FIELD(value, json, transaction_hash);
//    RPCAPI_DESERIALIZE_FIELD(value, json, proof);

//    return value;
//}


/*static*/
CreateSendProof::Response
CreateSendProof::Response::fromJson(const QVariantMap& json)
{
    CreateSendProof::Response value;

    RPCAPI_DESERIALIZE_FIELD(value, json, sendproofs);

    return value;
}

QVariantMap
CreateSendProof::Request::toJson() const
{
    const CreateSendProof::Request& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, transaction_hash);
    RPCAPI_SERIALIZE_FIELD(value, json, message);
    RPCAPI_SERIALIZE_FIELD(value, json, addresses);

    return json;
}

/*static*/
CheckSendProof::Response
CheckSendProof::Response::fromJson(const QVariantMap& json)
{
    CheckSendProof::Response value;

//    RPCAPI_DESERIALIZE_FIELD(value, json, validation_error);
    RPCAPI_DESERIALIZE_FIELD(value, json, transaction_hash);
    RPCAPI_DESERIALIZE_FIELD(value, json, address);
    RPCAPI_DESERIALIZE_FIELD(value, json, amount);
    RPCAPI_DESERIALIZE_FIELD(value, json, message);
    RPCAPI_DESERIALIZE_FIELD(value, json, output_indexes);

    return value;
}

QVariantMap
CheckSendProof::Request::toJson() const
{
    const CheckSendProof::Request& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, sendproof);

    return json;
}

QVariantMap
SetAddressLabel::Request::toJson() const
{
    const SetAddressLabel::Request& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, address);
    RPCAPI_SERIALIZE_FIELD(value, json, label);

    return json;
}

QVariantMap
GetWalletRecords::Request::toJson() const
{
    const GetWalletRecords::Request& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, need_secrets);
    RPCAPI_SERIALIZE_FIELD(value, json, create);
    RPCAPI_SERIALIZE_FIELD(value, json, index);
    RPCAPI_SERIALIZE_FIELD(value, json, count);

    return json;
}

/*static*/
GetWalletRecords::Response
GetWalletRecords::Response::fromJson(const QVariantMap& json)
{
    GetWalletRecords::Response value;

    RPCAPI_DESERIALIZE_LIST(value, json, records);
    RPCAPI_DESERIALIZE_FIELD(value, json, total_count);

    return value;
}

QVariantMap
CreateAddresses::Request::toJson() const
{
    const CreateAddresses::Request& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, secret_spend_keys);
    RPCAPI_SERIALIZE_TIMESTAMP(value, json, creation_timestamp);

    return json;
}

/*static*/
CreateAddresses::Response
CreateAddresses::Response::fromJson(const QVariantMap& json)
{
    CreateAddresses::Response value;

    RPCAPI_DESERIALIZE_FIELD(value, json, addresses);
    RPCAPI_DESERIALIZE_FIELD(value, json, secret_spend_keys);

    return value;
}

#undef RPCAPI_SERIALIZE_FIELD
#undef RPCAPI_SERIALIZE_TIMESTAMP
#undef RPCAPI_SERIALIZE_STRUCT
#undef RPCAPI_SERIALIZE_LIST

#undef RPCAPI_DESERIALIZE_FIELD
#undef RPCAPI_DESERIALIZE_TIMESTAMP
#undef RPCAPI_DESERIALIZE_LIST
#undef RPCAPI_DESERIALIZE_STRUCT

}
