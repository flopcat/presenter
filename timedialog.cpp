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
#include "timedialog.h"
#include "ui_timedialog.h"

TimeDialog::TimeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimeDialog)
{
    ui->setupUi(this);
}

TimeDialog::~TimeDialog()
{
    delete ui;
}

void TimeDialog::updateCountdown() const
{
    c->dayOfWeek = ui->dayOfWeek->currentIndex() + 1;
    c->endTime = ui->endTime->time();
    c->duration = ui->duration->time();
    c->updateTimer();
}

void TimeDialog::setCountdown(QSharedPointer<Countdown> &c)
{
    this->c = c;
    ui->dayOfWeek->setCurrentIndex(c->dayOfWeek - 1);
    ui->endTime->setTime(c->endTime);
    ui->duration->setTime(c->duration);
}
