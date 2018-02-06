#include <QVariantMap>

#include "rpcapi.h"

namespace RpcApi
{


constexpr char GetStatus::METHOD[];
constexpr char GetAddresses::METHOD[];
constexpr char GetTransfers::METHOD[];
constexpr char GetViewKey::METHOD[];
constexpr char GetBalance::METHOD[];
constexpr char GetUnspents::METHOD[];
constexpr char CreateTransaction::METHOD[];
constexpr char SendTransaction::METHOD[];

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
    RPCAPI_DESERIALIZE_FIELD(value, json, top_block_height);
    RPCAPI_DESERIALIZE_FIELD(value, json, top_block_difficulty);

    RPCAPI_DESERIALIZE_TIMESTAMP(value, json, top_block_timestamp);
    RPCAPI_DESERIALIZE_TIMESTAMP(value, json, top_block_timestamp_median);

    RPCAPI_DESERIALIZE_FIELD(value, json, next_block_effective_median_size);
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

    return json;
}

/*static*/
GetAddresses::Response
GetAddresses::Response::fromJson(const QVariantMap& json)
{
    GetAddresses::Response value;

    RPCAPI_DESERIALIZE_FIELD(value, json, addresses);
    RPCAPI_DESERIALIZE_FIELD(value, json, view_only);

    return value;
}

/*static*/
GetViewKey::Response
GetViewKey::Response::fromJson(const QVariantMap& json)
{
    GetViewKey::Response value;

    RPCAPI_DESERIALIZE_FIELD(value, json, secret_view_key);
    RPCAPI_DESERIALIZE_FIELD(value, json, public_view_key);

    return value;
}

/*static*/
GetBalance::Response
GetBalance::Response::fromJson(const QVariantMap& json)
{
    GetBalance::Response value;

    RPCAPI_DESERIALIZE_FIELD(value, json, spendable);
    RPCAPI_DESERIALIZE_FIELD(value, json, spendable_dust);
    RPCAPI_DESERIALIZE_FIELD(value, json, locked_or_unconfirmed);

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

    return json;
}

/*static*/
CreateTransaction::Response
CreateTransaction::Response::fromJson(const QVariantMap& json)
{
    CreateTransaction::Response value;

    RPCAPI_DESERIALIZE_FIELD(value, json, binary_transaction);
    RPCAPI_DESERIALIZE_FIELD(value, json, save_history_error);

    RPCAPI_DESERIALIZE_STRUCT(value, json, transaction);

    return value;
}

QVariantMap
CreateTransaction::Request::toJson() const
{
    const CreateTransaction::Request& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_STRUCT(value, json, transaction);

    RPCAPI_SERIALIZE_FIELD(value, json, spend_address);
    RPCAPI_SERIALIZE_FIELD(value, json, any_spend_address);
    RPCAPI_SERIALIZE_FIELD(value, json, change_address);
    RPCAPI_SERIALIZE_FIELD(value, json, confirmed_height_or_depth);
    RPCAPI_SERIALIZE_FIELD(value, json, fee_per_byte);
    RPCAPI_SERIALIZE_FIELD(value, json, optimization);
    RPCAPI_SERIALIZE_FIELD(value, json, save_history);

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
    RPCAPI_DESERIALIZE_FIELD(value, json, global_index);
    RPCAPI_DESERIALIZE_FIELD(value, json, unlock_time);
    RPCAPI_DESERIALIZE_FIELD(value, json, index_in_transaction);
    RPCAPI_DESERIALIZE_FIELD(value, json, height);
    RPCAPI_DESERIALIZE_FIELD(value, json, key_image);
    RPCAPI_DESERIALIZE_FIELD(value, json, transaction_public_key);
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
    RPCAPI_SERIALIZE_FIELD(value, json, global_index);
    RPCAPI_SERIALIZE_FIELD(value, json, unlock_time);
    RPCAPI_SERIALIZE_FIELD(value, json, index_in_transaction);
    RPCAPI_SERIALIZE_FIELD(value, json, height);
    RPCAPI_SERIALIZE_FIELD(value, json, key_image);
    RPCAPI_SERIALIZE_FIELD(value, json, transaction_public_key);
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

    RPCAPI_DESERIALIZE_LIST(value, json, outputs);

    return value;
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
    RPCAPI_DESERIALIZE_FIELD(value, json, nonce);

    RPCAPI_DESERIALIZE_FIELD(value, json, height);
    RPCAPI_DESERIALIZE_FIELD(value, json, hash);
    RPCAPI_DESERIALIZE_FIELD(value, json, reward);
    RPCAPI_DESERIALIZE_FIELD(value, json, cumulative_difficulty);
    RPCAPI_DESERIALIZE_FIELD(value, json, difficulty);
    RPCAPI_DESERIALIZE_FIELD(value, json, base_reward);
    RPCAPI_DESERIALIZE_FIELD(value, json, block_size);
    RPCAPI_DESERIALIZE_FIELD(value, json, transactions_cumulative_size);
    RPCAPI_DESERIALIZE_FIELD(value, json, already_generated_coins);
    RPCAPI_DESERIALIZE_FIELD(value, json, already_generated_transactions);
    RPCAPI_DESERIALIZE_FIELD(value, json, size_median);
    RPCAPI_DESERIALIZE_FIELD(value, json, effective_size_median);
    RPCAPI_DESERIALIZE_TIMESTAMP(value, json, timestamp_median);
    RPCAPI_DESERIALIZE_TIMESTAMP(value, json, timestamp_unlock);
    RPCAPI_DESERIALIZE_FIELD(value, json, total_fee_amount);

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

QVariantMap
Transfer::toJson() const
{
    const Transfer& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, address);
    RPCAPI_SERIALIZE_FIELD(value, json, amount);
    RPCAPI_SERIALIZE_FIELD(value, json, ours);

    RPCAPI_SERIALIZE_LIST(value, json, outputs);

    return json;
}

/*static*/
Transaction
Transaction::fromJson(const QVariantMap& json)
{
    Transaction value;

    RPCAPI_DESERIALIZE_FIELD(value, json, unlock_time);
    RPCAPI_DESERIALIZE_FIELD(value, json, payment_id);
    RPCAPI_DESERIALIZE_FIELD(value, json, anonymity);
    RPCAPI_DESERIALIZE_FIELD(value, json, hash);
    RPCAPI_DESERIALIZE_FIELD(value, json, fee);
    RPCAPI_DESERIALIZE_FIELD(value, json, public_key);
    RPCAPI_DESERIALIZE_FIELD(value, json, extra);
    RPCAPI_DESERIALIZE_FIELD(value, json, coinbase);
    RPCAPI_DESERIALIZE_FIELD(value, json, amount);
    RPCAPI_DESERIALIZE_FIELD(value, json, block_height);
    RPCAPI_DESERIALIZE_FIELD(value, json, block_hash);

    RPCAPI_DESERIALIZE_LIST(value, json, transfers);
    RPCAPI_DESERIALIZE_TIMESTAMP(value, json, timestamp);

    return value;
}

QVariantMap
Transaction::toJson() const
{
    const Transaction& value = *this;
    QVariantMap json;

    RPCAPI_SERIALIZE_FIELD(value, json, unlock_time);
    RPCAPI_SERIALIZE_FIELD(value, json, payment_id);
    RPCAPI_SERIALIZE_FIELD(value, json, anonymity);
    RPCAPI_SERIALIZE_FIELD(value, json, hash);
    RPCAPI_SERIALIZE_FIELD(value, json, fee);
    RPCAPI_SERIALIZE_FIELD(value, json, public_key);
    RPCAPI_SERIALIZE_FIELD(value, json, extra);
    RPCAPI_SERIALIZE_FIELD(value, json, coinbase);
    RPCAPI_SERIALIZE_FIELD(value, json, amount);
    RPCAPI_SERIALIZE_FIELD(value, json, block_height);
    RPCAPI_SERIALIZE_FIELD(value, json, block_hash);

    RPCAPI_SERIALIZE_LIST(value, json, transfers);
    RPCAPI_SERIALIZE_TIMESTAMP(value, json, timestamp);

    return json;
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
