// Copyright (c) 2015-2018, The Bytecoin developers.
// Licensed under the GNU Lesser General Public License. See LICENSE for details.

#ifndef POPUP_H
#define POPUP_H

#include <QLabel>
#include <QPropertyAnimation>
#include <QLayout>
#include <QTimer>

namespace WalletGUI
{

class MagicToolTip : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(float opacity READ opacity WRITE setOpacity)

public:
    explicit MagicToolTip(QWidget *parent = 0);

    float opacity() const;
    QString toolTip() const;

private:
    virtual void paintEvent(QPaintEvent *event) override;

public slots:
    void setToolTip(const QString& text);

    void setMinOpacity(float value);
    void setMaxOpacity(float value);
    void setShowAnimationDuration(int msecs);
    void setHideAnimationDuration(int msecs);
    void setToolTipDuration(int msecs);

    void show(const QPoint& point);
    void show(int x, int y);

private slots:
    void hideAnimation();
    void setOpacity(float value);

private:
    QLabel label_;
    QHBoxLayout layout_;
    QPropertyAnimation animation_;
    QTimer timer_;
    float opacity_;
    float minOpacity_;
    float maxOpacity_;
    int showDuration_;
    int hideDuration_;
    int toolTipDuration_;
};

class CopiedToolTip : public MagicToolTip
{
    Q_OBJECT

public:
    explicit CopiedToolTip(QWidget *parent = 0);

public slots:
    void showNearMouse();
};

}

#endif // POPUP_H

