// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include <QByteArray>

class QJsonObject;
class QJsonValue;

namespace JsonRpc {

class JsonRpcObject;

class JsonRpcObjectFactory {
public:
  static JsonRpcObject* createJsonRpcObject(const QJsonValue& _jsonValue, int& _jsonRpcErrorCode, QString& _jsonRpcErrorString,
    QString& _jsonRpcErrorData);

private:
  static JsonRpcObject* parseJsonRpcObject(const QJsonObject& _jsonObject, int& _jsonRpcErrorCode, QString& _jsonRpcErrorString,
  QString& _jsonRpcErrorData);
};

}
