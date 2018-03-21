// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QJsonArray>
#include <QJsonDocument>

#include "JsonRpcNotification.h"

namespace JsonRpc {

JsonRpcNotification::JsonRpcNotification() : JsonRpcObject(TYPE_NOTIFICATION) {
}

JsonRpcNotification::JsonRpcNotification(const QJsonObject& _jsonObject) : JsonRpcObject(_jsonObject, TYPE_NOTIFICATION) {
}

JsonRpcNotification::~JsonRpcNotification() {
}

QString JsonRpcNotification::getMethod() const {
  return getValue(methodTagName).toString();
}

QVariantList JsonRpcNotification::getParamsAsArray() const {
  return getValue(paramsTagName).toArray().toVariantList();
}

QVariantMap JsonRpcNotification::getParamsAsObject() const {
  return getValue(paramsTagName).toObject().toVariantMap();
}

void JsonRpcNotification::setMethod(const QString& _method) {
  setValue(methodTagName, _method);
}

void JsonRpcNotification::setParamsFromArray(const QVariantList& _variantList) {
  setValue(paramsTagName, QJsonArray::fromVariantList(_variantList));
}

void JsonRpcNotification::setParamsFromObject(const QVariantMap& _variantMap) {
  setValue(paramsTagName, QJsonObject::fromVariantMap(_variantMap));
}

}
