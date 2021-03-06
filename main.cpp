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
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("PresenterDevs");
    QCoreApplication::setApplicationName("Presenter");
    QApplication a(argc, argv);

    // mpv needs LC_NUMERIC set to the C locale
    setlocale(LC_NUMERIC, "C");

    a.setQuitOnLastWindowClosed(false);
    MainWindow w;
    if (w.startMinimized())
        w.showMinimized();
    else if (w.startVisible())
        w.show();
    return a.exec();
}
