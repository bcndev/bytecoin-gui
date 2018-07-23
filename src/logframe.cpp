// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include "logframe.h"
#include "ui_logframe.h"

#include <QFontDatabase>
#include <QScrollBar>
#include <QClipboard>
#include <QMutex>

namespace WalletGUI
{

static constexpr int MAX_LINES = 10000;
static constexpr int MAX_TEXT_SIZE = 1024 * 1024;

LogFrame::LogFrame(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::LogFrame)
    , mutex_(new QMutex)
    , autoScroll_(true)
{
    ui->setupUi(this);
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->textEdit->setFont(fixedFont);
    defaultColor_ = ui->textEdit->textColor();

    ui->textEdit->document()->setMaximumBlockCount(MAX_LINES);

    QScrollBar* verticalScrollBar = ui->textEdit->verticalScrollBar();
    connect(verticalScrollBar, &QScrollBar::valueChanged, this, &LogFrame::scrolled);
}

LogFrame::~LogFrame()
{
    delete mutex_;
    delete ui;
}

void LogFrame::scrolled(int value)
{
    const QScrollBar* verticalScrollBar = ui->textEdit->verticalScrollBar();
    autoScroll_ = (value == verticalScrollBar->maximum());
}

void LogFrame::scrollDown()
{
    QScrollBar* verticalScrollBar = ui->textEdit->verticalScrollBar();
    verticalScrollBar->setValue(verticalScrollBar->maximum());
}

void LogFrame::addDaemonOutput(const QString& data)
{
    print(data);
}

void LogFrame::addDaemonError(const QString& data)
{
    print(data, Qt::red);
}

void LogFrame::addGuiMessage(const QString& data)
{
    print(data, Qt::blue);
}

void LogFrame::addNetworkMessage(const QString& data)
{
    print(data, Qt::darkGreen);
}

void LogFrame::print(const QString& data, const QColor& color)
{
    QMutexLocker locker(mutex_);
    insertText(data, color);
    if (autoScroll_)
        scrollDown();
}

void LogFrame::insertText(const QString& data, const QColor& color)
{
    ui->textEdit->setTextColor(color.isValid() ? color : defaultColor_);
    if (data.size() > MAX_TEXT_SIZE)
        ui->textEdit->append(data.left(MAX_TEXT_SIZE));
    else
        ui->textEdit->append(data);
}

void LogFrame::copyToClipboard()
{
    QApplication::clipboard()->setText(ui->textEdit->toPlainText());
}

}
