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
#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QDateTime>
#include <QTimer>
#include <QWidget>

class DisplayWidget : public QWidget
{
    Q_OBJECT
    enum Displaying { DisplayingNothing, DisplayingCountdown, DisplayingImage };
    enum Fading { FadedOut, FadingIn, FadedIn, FadingOut };

public:
    explicit DisplayWidget(QWidget *parent = nullptr, bool widgetMode = false);
    void startCountdown(int msecDuration);
    void displayFile(const QString &filename);
    void stop();

signals:

public slots:

private slots:
    void timer_timeout();
    void fadeTimer_timeout();

protected:
    void paintEvent(QPaintEvent *e);

private:
    void paintNothing();
    void paintCountdown();
    void paintImage();
    void startFader(Fading effect);

private:
    Displaying displayMode;
    bool widgetMode;

    QTimer timer;
    qint64 msecLeft;
    qint64 msecDuration;
    QDateTime endTime;

    QDateTime fadeStart;
    QTimer fadeTimer;
    Fading fadeMode = FadedOut;
    double fadeFactor = 0.0;

    QImage image;
};

#endif // DISPLAYWIDGET_H
