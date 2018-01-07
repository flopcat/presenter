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
#ifndef COMMON_H
#define COMMON_H

#include <QDataStream>
#include <QTime>
#include <QTimer>
#include <QString>
#include <QLocale>
#include <QSettings>
#include <QVariant>
#include <QSharedPointer>

class Countdown : public QObject {
    Q_OBJECT

public:
    Countdown();
    Countdown(Countdown &c);
    ~Countdown();

    QString toString() const;
    qint64 remainingTimeInMsec();
    void updateTimer();

    int dayOfWeek = 1;
    QTime endTime;
    QTime duration;
    QSharedPointer<QTimer> timer;

    void readSettings(QSettings &settings);
    void writeSettings(QSettings &settings);
};

#endif // COMMON_H
