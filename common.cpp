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
#include <QDebug>
#include "common.h"

Countdown::Countdown() : QObject()
{
    dayOfWeek = 2;
    endTime = QTime(19,0,0);
    duration = QTime(0, 5, 0);
    timer.reset(new QTimer());
    timer->setSingleShot(true);
}

Countdown::Countdown(Countdown &c) : QObject()
{
    dayOfWeek = c.dayOfWeek;
    endTime = c.endTime;
    duration = c.duration;
    timer = c.timer;
}

Countdown::~Countdown()
{

}

QString Countdown::toString() const {
    return QString("%1: %2 [%3]").arg(QLocale().dayName(dayOfWeek),
                                      endTime.toString(),
                                      duration.toString());
}

void Countdown::updateTimer() {
    if (timer.isNull()) {
        qDebug() << "creating timer object in update timer";
        timer.reset(new QTimer());
    }
    QDateTime now(QDateTime::currentDateTime());
    int nowsec = (now.date().dayOfWeek()-1)*86400;
    nowsec += (now.time().msecsSinceStartOfDay()/1000);

    int futuresec = (dayOfWeek-1)*86400;
    futuresec += endTime.msecsSinceStartOfDay()/1000;
    futuresec -= duration.msecsSinceStartOfDay()/1000;

    int offset = futuresec - nowsec;
    if (offset < 10) // futuretime is past the end of week from nowtime
        offset += 60*60*24*7;
    timer->setInterval(offset*1000);
}

static const char settingDayOfWeek[] = "dayOfWeek";
static const char settingEndTime[] = "endTime";
static const char settingDuration[] = "duration";

void Countdown::readSettings(QSettings &settings)
{
    dayOfWeek = settings.value(settingDayOfWeek, 1).toInt();
    endTime = settings.value(settingEndTime, QTime(19,30,0)).toTime();
    duration = settings.value(settingDuration, QTime(0,5,0)).toTime();
}

void Countdown::writeSettings(QSettings &settings)
{
    settings.setValue(settingDayOfWeek, dayOfWeek);
    settings.setValue(settingEndTime, endTime);
    settings.setValue(settingDuration, duration);
}
