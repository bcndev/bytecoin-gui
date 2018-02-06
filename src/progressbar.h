#ifndef PROGRESSBAR
#define PROGRESSBAR

#include <QProgressBar>

class QAbstractItemModel;

namespace WalletGUI {

class WalletModel;

class WalletProgressBar : public QProgressBar
{
    Q_OBJECT
    Q_DISABLE_COPY(WalletProgressBar)

public:
    explicit WalletProgressBar(QWidget* parent);
    virtual ~WalletProgressBar();

    void setWalletModel(WalletModel* model);

private:
    WalletModel* walletModel_;

    void nodeStateChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void updateMinimum(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);
    void updateSyncState();
};

}
#endif // PROGRESSBAR

