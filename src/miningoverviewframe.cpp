#include <QDataWidgetMapper>

#include "miningoverviewframe.h"
#include "ui_miningoverviewframe.h"
#include "MiningManager.h"
#include "MinerModel.h"
#include "walletmodel.h"

namespace WalletGUI {

MiningOverviewFrame::MiningOverviewFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::MiningOverviewFrame)
    , miningManager_(nullptr)
    , miningMapper_(new QDataWidgetMapper(this))
    , started_(false)
{
    ui->setupUi(this);

    QFont font = ui->m_overviewHashRateLabel->font();
    font.setBold(true);
    ui->m_overviewHashRateLabel->setFont(font);
    ui->m_miningStateLabel->setFont(font);
    ui->m_overviewNetworkHashrateLabel->setFont(font);
    ui->m_overviewNetworkDifficultyLabel->setFont(font);
}

MiningOverviewFrame::~MiningOverviewFrame()
{
    delete ui;
}

void MiningOverviewFrame::setMiningManager(MiningManager* miningManager)
{
    miningManager_ = miningManager;

    connect(miningManager_, &MiningManager::miningStartedSignal, this, &MiningOverviewFrame::miningStarted);
    connect(miningManager_, &MiningManager::miningStoppedSignal, this, &MiningOverviewFrame::miningStopped);
    connect(miningManager_, &MiningManager::activeMinerChangedSignal, this, &MiningOverviewFrame::activeMinerChanged);
}

void MiningOverviewFrame::setMinerModel(QAbstractItemModel* model)
{
    miningMapper_->setModel(model);
    miningMapper_->addMapping(ui->m_overviewHashRateLabel, MinerModel::COLUMN_HASHRATE, "text");
    if (model->rowCount() > 0)
        miningMapper_->setCurrentIndex(0);
    else
        ui->m_overviewHashRateLabel->setText("0 H/s");
}

void MiningOverviewFrame::setWalletModel(QAbstractItemModel* model)
{
    QDataWidgetMapper* stateMapper = new QDataWidgetMapper(this);
    stateMapper->setModel(model);
    stateMapper->addMapping(ui->m_overviewNetworkHashrateLabel, WalletModel::COLUMN_NETWORK_HASHRATE, "text");
    stateMapper->addMapping(ui->m_overviewNetworkDifficultyLabel, WalletModel::COLUMN_TOP_BLOCK_DIFFICULTY, "text");
    stateMapper->toFirst();
    connect(model, &QAbstractItemModel::modelReset, stateMapper, &QDataWidgetMapper::toFirst);
}

void MiningOverviewFrame::startMiningClicked()
{
    if (started_)
    {
        miningStopped();
        miningManager_->stopMining();
    }
    else
    {
        miningStarted();
        miningManager_->startMining();
    }
}

void MiningOverviewFrame::miningStarted()
{
    ui->m_miningStateLabel->setText(tr("ON"));
    started_ = true;
}

void MiningOverviewFrame::miningStopped()
{
    ui->m_miningStateLabel->setText(tr("OFF"));
    started_ = false;
}

void MiningOverviewFrame::activeMinerChanged(quintptr _minerIndex)
{
    miningMapper_->setCurrentIndex(_minerIndex);
}

}
