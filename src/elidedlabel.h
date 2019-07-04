#ifndef ELLIDEDLABEL_H
#define ELLIDEDLABEL_H

#include <QLabel>

namespace WalletGUI {

class ElidedLabel : public QLabel
{
    Q_OBJECT
public:
    using QLabel::QLabel;
    void setElideMode(Qt::TextElideMode elideMode);
    Qt::TextElideMode elideMode() const { return m_elideMode; }

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;

private:
    void updateCachedTexts();

private:
    Qt::TextElideMode m_elideMode = Qt::ElideMiddle;
    QString m_cachedElidedText;
    QString m_cachedText;
};

}

#endif // ELLIDEDLABEL_H
