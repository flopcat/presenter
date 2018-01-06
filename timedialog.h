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
#ifndef TIMEDIALOG_H
#define TIMEDIALOG_H

#include <QDateTime>
#include <QDialog>
#include "common.h"

namespace Ui {
class TimeDialog;
}

class TimeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TimeDialog(QWidget *parent = 0);
    ~TimeDialog();
    void updateCountdown() const;
    void setCountdown(QSharedPointer<Countdown> &c);

private:
    Ui::TimeDialog *ui;
    QSharedPointer<Countdown> c;
};

#endif // TIMEDIALOG_H
