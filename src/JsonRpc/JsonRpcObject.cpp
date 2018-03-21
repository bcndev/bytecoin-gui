// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QJsonDocument>

#include "JsonRpcObject.h"

namespace JsonRpc {

const char JsonRpcObject::JSON_RPC_PARSE_ERROR_MESSAGE[] = "Parse error";
const char JsonRpcObject::JSON_RPC_INVALID_REQUEST_ERROR_MESSAGE[] = "Invalid Request";
const char JsonRpcObject::JSON_RPC_METHOD_NOT_FOUND_ERROR_MESSAGE[] = "Method not found";

const QString JsonRpcObject::jsonRpcVersion = "2.0";
const QString JsonRpcObject::jsonRpcTagName = "jsonrpc";
const QString JsonRpcObject::idTagName = "id";
const QString JsonRpcObject::methodTagName = "method";
const QString JsonRpcObject::paramsTagName = "params";
const QString JsonRpcObject::resultTagName = "result";
const QString JsonRpcObject::errorTagName = "error";
const QString JsonRpcObject::errorCodeTagName = "code";
const QString JsonRpcObject::errorMessageTagName = "message";
const QString JsonRpcObject::errorDataTagName = "data";

JsonRpcObject::JsonRpcObject(JsonRpcObjectType _objectType) : m_jsonObject(), m_objectType(_objectType) {
  m_jsonObject.insert(jsonRpcTagName, jsonRpcVersion);
}

JsonRpcObject::JsonRpcObject(const QJsonObject& _jsonObject, JsonRpcObjectType _objectType) : m_jsonObject(_jsonObject),
  m_objectType(_objectType) {
  m_jsonObject.insert(jsonRpcTagName, jsonRpcVersion);
}

QJsonValue JsonRpcObject::getValue(const QString& _name) const {
  return m_jsonObject.value(_name);
}

bool JsonRpcObject::contains(const QString& _name) const {
  return m_jsonObject.contains(_name);
}

void JsonRpcObject::setValue(const QString& _name, const QJsonValue& _value) {
  m_jsonObject.insert(_name, _value);
}

JsonRpcObject::~JsonRpcObject() {
}

bool JsonRpcObject::isValid() const {
  switch (m_objectType) {
  case TYPE_REQUEST:
    return isValidJsonRpcRequest(m_jsonObject);
  case TYPE_RESPONSE:
    return isValidJsonRpcResponse(m_jsonObject);
  case TYPE_NOTIFICATION:
    return isValidJsonRpcNotification(m_jsonObject);
  }

  return false;
}

QByteArray JsonRpcObject::toString() const {
  return QJsonDocument(m_jsonObject).toJson(QJsonDocument::Compact);
}

QJsonObject JsonRpcObject::toJsonObject() const {
  return m_jsonObject;
}

QString JsonRpcObject::getJsonRpc() const {
  return m_jsonObject.value(jsonRpcTagName).toString();
}

bool JsonRpcObject::isRequest() const {
  return m_objectType == TYPE_REQUEST;
}

bool JsonRpcObject::isResponse() const {
  return m_objectType == TYPE_RESPONSE;
}

bool JsonRpcObject::isNotification() const {
  return m_objectType == TYPE_NOTIFICATION;
}

bool JsonRpcObject::isValidJsonRpcRequest(const QJsonObject& _jsonObject) {
  return !_jsonObject.value(jsonRpcTagName).toString().compare(jsonRpcVersion) && _jsonObject.contains(methodTagName) &&
      !_jsonObject.value(methodTagName).isNull() && _jsonObject.contains(idTagName);
}

bool JsonRpcObject::isValidJsonRpcResponse(const QJsonObject& _jsonObject) {
  bool isValid = !_jsonObject.value(jsonRpcTagName).toString().compare(jsonRpcVersion) && _jsonObject.contains(idTagName);
  if (isValid) {
//    isValid = (_jsonObject.contains(resultTagName) && !_jsonObject.contains(errorTagName)) ||
//      (!_jsonObject.contains(resultTagName) && _jsonObject.contains(errorTagName));
      isValid = (_jsonObject.contains(resultTagName) != _jsonObject.contains(errorTagName));
    if (isValid && _jsonObject.contains(errorTagName)) {
      isValid = _jsonObject.value(errorTagName).toObject().contains(errorCodeTagName) &&
        _jsonObject.value(errorTagName).toObject().contains(errorMessageTagName);
      if (isValid) {
        int id = _jsonObject.value(errorTagName).toObject().value(errorCodeTagName).toInt(std::numeric_limits<int>::max());
        if (id == std::numeric_limits<int>::max()) {
          isValid = false;
        }
      }
    }
  }

  return isValid;
}

bool JsonRpcObject::isValidJsonRpcNotification(const QJsonObject& _jsonObject) {
  return !_jsonObject.value(jsonRpcTagName).toString().compare(jsonRpcVersion) && _jsonObject.contains(methodTagName) &&
  !_jsonObject.value(methodTagName).isNull() && !_jsonObject.contains(idTagName);
}

}
