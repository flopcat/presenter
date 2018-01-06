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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListWidget>
#include <QMainWindow>
#include <QSettings>
#include <QSystemTrayIcon>
#include "common.h"
#include "displaywidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool startMinimized();

    void restoreSettings();
    void saveSettings();

public slots:
    void populateScreens();

protected:
    void closeEvent(QCloseEvent *event);

private:
    void setupTrayIcon();
    void setupScreens();
    void useDisplayGeometry();

    void appendCountdown(QSharedPointer<Countdown> c);
    void appendImages(const QStringList &images);
    void startCountdown(int msecDuration);
    void startImage(const QString &filename);

private slots:
    void on_countdownAdd_clicked();

    void on_countdownRemove_clicked();

    void on_countdownClear_clicked();

    void on_countdownTest_clicked();

    void on_countdownList_itemDoubleClicked(QListWidgetItem *item);

    void on_countdownStop_clicked();

    void on_imagesAdd_clicked();

    void on_imagesRemove_clicked();

    void on_imagesClear_clicked();

    void on_imagesShow_clicked();

    void on_imagesHide_clicked();

    void on_imagesList_itemDoubleClicked(QListWidgetItem *item);

    void on_imagesList_currentTextChanged(const QString &currentText);

    void on_monitorCombo_currentIndexChanged(int index);

    void on_actionHelpAboutPresenter_triggered();

    void on_actionHelpAboutQt_triggered();

    void on_actionHelpExit_triggered();

private:
    Ui::MainWindow *ui;
    QSystemTrayIcon icon;
    QSettings settings;
    DisplayWidget displayWidget;

    QList<QRect> screenAreas;
    QList<QSharedPointer<Countdown>> countdowns;

    QRect usedDisplayGeometry;
};

#endif // MAINWINDOW_H
