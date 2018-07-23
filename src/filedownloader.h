#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace WalletGUI {

class FileDownloader : public QObject
{
    Q_OBJECT
public:
    explicit FileDownloader(QObject *parent = 0);
    virtual ~FileDownloader();
    QByteArray downloadedData() const;

signals:
    void downloaded();

public slots:
    void download(QUrl fileUrl);

private slots:
    void fileDownloaded(QNetworkReply* reply);

private:
    QNetworkAccessManager netManager_;
    QByteArray data_;
};

}

#endif // FILEDOWNLOADER_H
