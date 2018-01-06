/* This file is part of Presenter.
 *
 * Presenter is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <QApplication>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPaintEvent>
#include <QTimer>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "timedialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupTrayIcon();
    setupScreens();
    restoreSettings();
}

MainWindow::~MainWindow()
{
    saveSettings();
    delete ui;
}

bool MainWindow::startMinimized()
{
    return ui->programStartMinimized->isChecked();
}

static const char settingDisplayGeometry[] = "displayGeometry";
static const char settingStartMinimized[] = "startMinimized";
static const char settingWarnOnClose[] = "warnOnClose";
static const char settingCountdowns[] = "Countdowns";
static const char settingImages[] = "Images";
static const char settingFilename[] = "filename";

void MainWindow::restoreSettings()
{
    int index;
    int size;
    QStringList files;

    usedDisplayGeometry = settings.value(settingDisplayGeometry, QRect()).toRect();
    index = screenAreas.indexOf(usedDisplayGeometry);
    if (index >= 0)
        ui->monitorCombo->setCurrentIndex(index);

    ui->programStartMinimized->setChecked(settings.value(settingStartMinimized, false).toBool());
    ui->programWarnOnClose->setChecked(settings.value(settingWarnOnClose, true).toBool());

    size = settings.beginReadArray(settingCountdowns);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QSharedPointer<Countdown> c(new Countdown);
        c->readSettings(settings);
        c->updateTimer();
        auto cData = c.data();
        connect(c->timer.data(), &QTimer::timeout,
                cData, [this,cData]() {
            startCountdown(cData->duration.msecsSinceStartOfDay());
            cData->updateTimer();
            cData->timer->start();
        });
        c->timer->start();
        appendCountdown(c);
    }
    settings.endArray();

    size = settings.beginReadArray(settingImages);
    files.clear();
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        files.append(settings.value(settingFilename).toString());
    }
    settings.endArray();
    appendImages(files);
}

void MainWindow::saveSettings()
{
    int size;

    settings.setValue(settingDisplayGeometry, usedDisplayGeometry);
    settings.setValue(settingStartMinimized, ui->programStartMinimized->isChecked());
    settings.setValue(settingWarnOnClose, ui->programWarnOnClose->isChecked());

    size = countdowns.size();
    settings.beginWriteArray(settingCountdowns);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        countdowns[i]->writeSettings(settings);
    }
    settings.endArray();

    size = ui->imagesList->count();
    settings.beginWriteArray(settingImages);
    for (int i = 0; i < size; ++i) {
       settings.setArrayIndex(i);
       settings.setValue(settingFilename, ui->imagesList->item(i)->text());
    }
    settings.endArray();
}

void MainWindow::populateScreens()
{
    ui->monitorCombo->clear();
    screenAreas.clear();
    QDesktopWidget *desktop = QApplication::desktop();
    for (int i = 0; i < desktop->screenCount(); i++) {
        QWidget *w = desktop->screen(i);
        QString s("%1: %2x%3+%4+%5");
        QRect g = w->geometry();
        screenAreas.append(g);
        s = s.arg(QString::number(i),
                  QString::number(g.width()), QString::number(g.height()),
                  QString::number(g.left()), QString::number(g.top()));
        ui->monitorCombo->addItem(s, QVariant(geometry()));
        if (g == usedDisplayGeometry)
            ui->monitorCombo->setCurrentIndex(i);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    if (ui->programWarnOnClose->isChecked()) {
        QString text("<h3>Deferred action</h3>"
                     "This application has an icon in the system tray."
                     "<p>The application will run in the background until "
                     "the quit action is triggered from the tray icon menu.");
        QMessageBox::information(this, "Tray icon in use - Presenter", text);
    }
    event->accept();
}


void MainWindow::setupTrayIcon()
{
    QMenu *m = new QMenu(this);
    connect(m->addAction(tr("&Show")),
            &QAction::triggered,
            this, &MainWindow::show);
    connect(m->addAction(tr("&Quit")),
            &QAction::triggered,
            qApp, &QCoreApplication::quit);
    icon.setIcon(QIcon(":/images/icon.svg"));
    icon.setContextMenu(m);
    icon.show();
}

void MainWindow::setupScreens()
{
    populateScreens();
    QDesktopWidget *desktop = QApplication::desktop();
    connect(desktop, &QDesktopWidget::resized,
            this, &MainWindow::populateScreens);
    connect(desktop, &QDesktopWidget::screenCountChanged,
            this, &MainWindow::populateScreens);
}

void MainWindow::useDisplayGeometry()
{
    if (!screenAreas.contains(usedDisplayGeometry))
        usedDisplayGeometry = screenAreas[ui->monitorCombo->currentIndex()];
    displayWidget.setGeometry(usedDisplayGeometry);
}

void MainWindow::appendCountdown(QSharedPointer<Countdown> c)
{
    ui->countdownList->addItem(c->toString());
    countdowns.append(c);
}

void MainWindow::appendImages(const QStringList &images)
{
    for (auto filename : images)
        ui->imagesList->addItem(filename);
}

void MainWindow::startCountdown(int msecDuration)
{
    useDisplayGeometry();
    displayWidget.startCountdown(msecDuration);
}

void MainWindow::startImage(const QString &filename)
{
    useDisplayGeometry();
    displayWidget.displayFile(filename);
}

void MainWindow::on_countdownAdd_clicked()
{
    TimeDialog d;
    QSharedPointer<Countdown> c(new Countdown);
    d.setCountdown(c);
    if(d.exec() == QDialog::Rejected)
        return;
    d.updateCountdown();
    Countdown *cData = c.data();
    connect(c->timer.data(), &QTimer::timeout,
            cData, [this,cData]() {
        startCountdown(cData->duration.msecsSinceStartOfDay());
        cData->updateTimer();
        cData->timer->start();
    });
    c->timer->start();
    appendCountdown(c);
}

void MainWindow::on_countdownRemove_clicked()
{
    int i = ui->countdownList->currentRow();
    if (i < 0)
        return;
    delete ui->countdownList->takeItem(i);
    countdowns.removeAt(i);
}

void MainWindow::on_countdownClear_clicked()
{
    ui->countdownList->clear();
    countdowns.clear();
}

void MainWindow::on_countdownTest_clicked()
{
    startCountdown(10000);
}

void MainWindow::on_countdownList_itemDoubleClicked(QListWidgetItem *item)
{
    Q_UNUSED(item);
    int i = ui->countdownList->currentRow();
    if (i < 0)
        return;
    TimeDialog d;
    d.setCountdown(countdowns[i]);
    if(d.exec() == QDialog::Rejected)
        return;
    d.updateCountdown();
    item->setText(countdowns[i]->toString());
}

void MainWindow::on_countdownStop_clicked()
{
    displayWidget.stop();
}

void MainWindow::on_imagesAdd_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(this, QString(), QString(), "Images (*.png;*.jpg;*.jpeg;*.svg");
    appendImages(files);
}

void MainWindow::on_imagesRemove_clicked()
{
    auto selected = ui->imagesList->selectedItems();
    for (auto i : selected)
        delete i;
}

void MainWindow::on_imagesClear_clicked()
{
    ui->imagesList->clear();
}

void MainWindow::on_imagesShow_clicked()
{
    auto item = ui->imagesList->currentItem();
    if (item == nullptr)
        return;
    startImage(item->text());
}

void MainWindow::on_imagesHide_clicked()
{
    displayWidget.stop();
}

void MainWindow::on_imagesList_itemDoubleClicked(QListWidgetItem *item)
{
    startImage(item->text());
}

void MainWindow::on_imagesList_currentTextChanged(const QString &currentText)
{
    QGraphicsScene scene;
    scene.addPixmap(QPixmap(currentText));
    ui->imagesPreview->setScene(&scene);
}

void MainWindow::on_monitorCombo_currentIndexChanged(int index)
{
    if (screenAreas.isEmpty() || index < 0)
        return;
    usedDisplayGeometry = screenAreas[index];
    displayWidget.setGeometry(usedDisplayGeometry);
}

void MainWindow::on_actionHelpAboutPresenter_triggered()
{
    QString text("\
<h3>Presenter</h3>\n\
Copyright (C) 2017%1%2 The Presenter Developers\n\
\n\
<p>This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\
\
<p>This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
GNU General Public License for more details.\n\
\n\
<p>You should have received a copy of the GNU General Public License along\n\
with this program; if not, write to the Free Software Foundation, Inc.,\n\
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.");
    QString yearDash, yearText;
    int nowyear = QDateTime::currentDateTime().date().year();
    if(nowyear > 2017) {
        yearDash = "-";
        yearText = QString::number(nowyear);
    }
    QMessageBox::about(this, tr("About Presenter"), text.arg(yearDash, yearText));
}

void MainWindow::on_actionHelpAboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionHelpExit_triggered()
{
    qApp->quit();
}
