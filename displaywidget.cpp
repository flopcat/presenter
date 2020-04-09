/* This file is part of Presenter.
 *
 * Presenter is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Presenter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with Presenter; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <QCoreApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QStyle>
#include <QTime>
#include "displaywidget.h"

constexpr qint64 fadeTimeMsec = 300;
constexpr qint64 updateMsec = 1000/20;

DisplayWidget::DisplayWidget(QWidget *parent, bool widgetMode) : QWidget(parent), widgetMode(widgetMode)
{
    displayMode = DisplayingNothing;
    fadeMode = FadedOut;
    if (!widgetMode)
        setWindowFlags(Qt::FramelessWindowHint | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint);

    timer.setInterval(updateMsec);
    timer.setSingleShot(false);
    fadeTimer.setInterval(updateMsec);
    fadeTimer.setSingleShot(false);
    connect(&timer, &QTimer::timeout,
            this, &DisplayWidget::timer_timeout);
    connect(&fadeTimer, &QTimer::timeout,
            this, &DisplayWidget::fadeTimer_timeout);
}

void DisplayWidget::startCountdown(int msecDuration)
{
    startCountdownPartway(0, msecDuration);
}

void DisplayWidget::startCountdownPartway(int msecPosition, int msecDuration)
{
    displayMode = DisplayingCountdown;
    QDateTime nowTime = QDateTime::currentDateTime();
    endTime = nowTime.addMSecs(msecDuration - msecPosition);
    this->msecDuration = msecDuration;
    msecLeft = msecDuration - msecPosition;
    timer.start();

    startFader(FadingIn);
    if (!widgetMode)
        show();
}

void DisplayWidget::displayFile(const QString &filename)
{
    bool success = image.load(filename);
    displayMode = success ? DisplayingImage : DisplayingNothing;
    startFader(FadingIn);
    update();
    if (!widgetMode)
        show();
}

void DisplayWidget::stop()
{
    startFader(FadingOut);
}

void DisplayWidget::timer_timeout()
{
    if (displayMode != DisplayingCountdown) {
        timer.stop();
        return;
    }

    QDateTime nowTime = QDateTime::currentDateTime();
    msecLeft = nowTime.msecsTo(endTime);
    update();

    if (msecLeft < 0 && !fadeTimer.isActive()) {
        startFader(FadingOut);
    }
}

void DisplayWidget::fadeTimer_timeout()
{
    if (widgetMode)
        return;

    if (fadeMode == FadedIn || fadeMode == FadedOut)
        return;

    QDateTime nowTime = QDateTime::currentDateTime();
    qint64 msecs = fadeStart.msecsTo(nowTime);
    qreal factor = msecs/double(fadeTimeMsec);
    factor = std::min(1.0, std::max(factor, 0.0));
    if (fadeFactor > factor)
        factor = fadeFactor;
    else
        fadeFactor = factor;

    setWindowOpacity(fadeMode == FadingIn ? factor : 1.0 - factor);
    update();

    if (fadeFactor >= 1.0) {
        if (fadeMode == FadingOut) {
            displayMode = DisplayingNothing;
            fadeMode = FadedOut;
            timer.stop();
            hide();
        } else {
            fadeMode = FadedIn;
        }
        fadeTimer.stop();
    }
}

void DisplayWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    switch (displayMode) {
    case DisplayingNothing:
        paintNothing();
        break;
    case DisplayingCountdown:
        paintCountdown();
        break;
    case DisplayingImage:
        paintImage();
        break;
    }
}

void DisplayWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !widgetMode)
        stop();
    QWidget::mousePressEvent(event);
}

void DisplayWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && !widgetMode)
        stop();
    QWidget::keyPressEvent(event);
}

void DisplayWidget::paintNothing()
{
    QColor bgColor(0,0,0);
    QPainter p(this);
    p.setBackground(bgColor);
    p.eraseRect(rect());
}

void DisplayWidget::paintCountdown()
{
    QColor fillColor(0xff,0xff,0xba);
    QColor backColor(0x49,0x49,0x63);
    QColor bgColor(0,0,0);

    int w = width();
    int h = height();
    int d = std::min(w,h);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRect bgRect { 0, 0, width(), height() };
    p.setBackground(bgColor);
    p.eraseRect(bgRect);
    p.translate(w > h ? QPoint((w-h)/2,0) : QPoint(0,(h-w)/2));

    QFont f;
    f.setFixedPitch(true);
    f.setBold(true);
    f.setPixelSize(d * 0.2);
    p.setFont(f);

    int time = std::min(std::max((msecLeft + (updateMsec/2)) / 1000, 0ll), 3599ll);
    QString text = QTime(0, time/60, time%60).toString("m:ss");
    QFontMetrics metrics(f);
    QSize sz = metrics.size(Qt::TextSingleLine, text);

    p.setPen(fillColor);
    QRect textRect(d * 0.4, (d - sz.height())/2, d*0.6, sz.height());
    p.drawText(textRect, Qt::AlignVCenter | Qt::AlignRight, text);

    QPointF pieCenter(d*0.2, h*0.5);
    double radius = d * 0.2;
    double radius2 = radius * 0.6;
    double factor = std::min(std::max(msecLeft/(double)msecDuration, 0.0), 1.0);
    QRectF pieRect(pieCenter - QPoint(radius,radius),
                   pieCenter + QPoint(radius,radius));
    p.setPen(Qt::NoPen);
    p.setBrush(backColor);
    p.drawEllipse(pieRect);
    p.setBrush(fillColor);
    p.drawPie(pieRect, 90*16, factor*360*16);
    p.setBrush(bgColor);
    p.drawEllipse(QRectF(pieCenter - QPoint(radius2,radius2),
                         pieCenter + QPoint(radius2,radius2)));
}

void DisplayWidget::paintImage()
{
    QSize picSize = image.size().scaled(size(), Qt::KeepAspectRatio);
    QRect picRect = QStyle::alignedRect(Qt::LayoutDirectionAuto,
                                        Qt::AlignCenter, picSize, rect());
    QColor bgColor(0,0,0);
    QPainter p(this);
    p.setBackground(bgColor);
    p.eraseRect(rect());
    p.drawImage(picRect, image);
}

void DisplayWidget::startFader(Fading effect)
{
    if (widgetMode)
        return;

    Fading priorMode = fadeMode;
    if ((priorMode == FadedIn && effect == FadingIn) ||
        (priorMode == FadedOut && effect == FadingOut))
        return;

    if (priorMode == FadedOut && effect == FadingIn)
        setWindowOpacity(0.0);
    if (priorMode == FadedIn && effect == FadingOut)
        setWindowOpacity(1.0);
    if ((priorMode == FadingIn && effect == FadingOut) ||
        (priorMode == FadingOut && effect == FadingIn)) {
        fadeFactor = 1.0 - fadeFactor;
    } else {
        fadeFactor = 0.0;
    }
    fadeStart = QDateTime::currentDateTime();
    fadeMode = effect;
    fadeTimer.start();
}
