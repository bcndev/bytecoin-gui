#include "filedownloader.h"

namespace WalletGUI {

FileDownloader::FileDownloader(QObject *parent)
    : QObject(parent)
{
    connect(&netManager_, &QNetworkAccessManager::finished, this, &FileDownloader::fileDownloaded);
}

FileDownloader::~FileDownloader()
{}

void FileDownloader::download(QUrl fileUrl)
{
    QNetworkRequest request(fileUrl);
    netManager_.get(request);
}

void FileDownloader::fileDownloaded(QNetworkReply* reply)
{
    data_ = reply->readAll();
    reply->deleteLater();
    emit downloaded();
}

QByteArray FileDownloader::downloadedData() const
{
    return data_;
}

}
