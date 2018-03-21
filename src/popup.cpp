// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#include "popup.h"

#include <QPainter>

namespace WalletGUI
{

MagicToolTip::MagicToolTip(QWidget *parent)
    : QWidget(parent)
    , opacity_(0.)
    , minOpacity_(0.)
    , maxOpacity_(1.)
    , showDuration_(300)
    , hideDuration_(700)
    , toolTipDuration_(1000)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    animation_.setTargetObject(this);
    animation_.setPropertyName("opacity");

    label_.setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    label_.setStyleSheet("QLabel { color : white; }");
//                        "margin-top: 6px;"
//                        "margin-bottom: 6px;"
//                        "margin-left: 10px;"
//                        "margin-right: 10px; }");

    layout_.addWidget(&label_);
    setLayout(&layout_);

    connect(&timer_, &QTimer::timeout, this, &MagicToolTip::hideAnimation);
}

void MagicToolTip::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect roundedRect;
    roundedRect.setX(rect().x() + 5);
    roundedRect.setY(rect().y() + 5);
    roundedRect.setWidth(rect().width() - 10);
    roundedRect.setHeight(rect().height() - 10);

    painter.setBrush(QBrush(QColor(0xEE4486))); // TODO: make setter
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(roundedRect, 7, 7);
}

void MagicToolTip::setToolTip(const QString &text)
{
    label_.setText(text);
    adjustSize();
}

QString MagicToolTip::toolTip() const
{
    return label_.text();
}

void MagicToolTip::show(const QPoint& point)
{
    show(point.x(), point.y());
}

void MagicToolTip::show(int x, int y)
{
    disconnect(&animation_, &QAbstractAnimation::finished, 0, 0);

    setWindowOpacity(opacity_);
    animation_.setDuration(showDuration_);
    animation_.setStartValue(minOpacity_);
    animation_.setEndValue(maxOpacity_);

    setGeometry(x + 20, y, width(), height());
    QWidget::show();

    animation_.start();
    timer_.start(toolTipDuration_);
}

void MagicToolTip::hideAnimation()
{
    connect(&animation_, &QAbstractAnimation::finished, this, &MagicToolTip::hide);

    timer_.stop();
    animation_.setDuration(hideDuration_);
    animation_.setStartValue(maxOpacity_);
    animation_.setEndValue(minOpacity_);
    animation_.start();
}

void MagicToolTip::setOpacity(float value)
{
    opacity_ = value;
    setWindowOpacity(opacity_);
}

float MagicToolTip::opacity() const
{
    return opacity_;
}

void MagicToolTip::setMinOpacity(float value)
{
    minOpacity_ = value;
    if (opacity_ < minOpacity_)
        opacity_ = minOpacity_;
}

void MagicToolTip::setMaxOpacity(float value)
{
    maxOpacity_ = value;
    if (opacity_ > maxOpacity_)
        opacity_ = maxOpacity_;
}

void MagicToolTip::setShowAnimationDuration(int msecs)
{
    showDuration_ = msecs;
}

void MagicToolTip::setHideAnimationDuration(int msecs)
{
    hideDuration_ = msecs;
}

void MagicToolTip::setToolTipDuration(int msecs)
{
    toolTipDuration_ = msecs;
}


CopiedToolTip::CopiedToolTip(QWidget *parent)
    : MagicToolTip(parent)
{
    setToolTip(tr("Copied"));
    hide();
}

void CopiedToolTip::showNearMouse()
{
    show(QCursor::pos());
}

}
