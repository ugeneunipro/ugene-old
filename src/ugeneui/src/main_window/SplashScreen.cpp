/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "SplashScreen.h"

#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/Version.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/qcoreevent.h>
#include <QtGui/QMovie>
#include <QtGui/QPainter>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#else
#include <QtWidgets/QLayout>
#include <QtWidgets/QLabel>
#endif

namespace U2{

#define TEXT_HEIGHT_PT 15
#define TEXT_PADDING_PT 12

SplashScreen::SplashScreen( QWidget *parent /* = NULL*/ ):QDialog(parent) {
    setEnabled(false);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
#ifdef Q_OS_WIN
    setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
#endif
    setWindowModality(Qt::ApplicationModal);
    QHBoxLayout* mainLayout = new QHBoxLayout();
    setLayout(mainLayout);
    setContentsMargins(0, 0, 0, 0);
    mainLayout->setMargin(0);
    QFrame *frame = new QFrame(this);
    mainLayout->addWidget(frame);

    QHBoxLayout* frameLayout = new QHBoxLayout();
    frameLayout->setMargin(0);
    frame->setContentsMargins(0, 0, 0, 0);
    frame->setLayout(frameLayout);

    SplashScreenWidget* sWidget = new SplashScreenWidget();

    QVBoxLayout* aWLayout = (QVBoxLayout*)frame->layout();
    aWLayout->insertWidget(0, sWidget);
    aWLayout->setStretchFactor(sWidget, 100);
    installEventFilter(this);
}

void SplashScreen::sl_close(){
    if(AppContext::getTaskScheduler() == qobject_cast<TaskScheduler*>(sender())){
        removeEventFilter(this);
        close();
    }
}

bool SplashScreen::eventFilter(QObject * /*obj*/, QEvent *ev){
    if(ev->type() == QEvent::Close){
        ev->ignore();
        return true;    
    }
    return false;
}

////////////////////////////////////////////////////////////////
SplashScreenWidget::SplashScreenWidget(){
    setObjectName("splash_screen_widget");

    QImage image(":ugene/images/ugene_splash.png");
    Version v = Version::appVersion();
    version = v.text;

    image1 = image2 = image;
    setFixedSize(image1.size());

    dots_number = 0;
    task = "";
    startTimer(500);

    getTask();
    getDots();
    drawInfo();
}

void SplashScreenWidget::getTask(){
    if(AppContext::getTaskScheduler() == NULL){
        return;
    }
    QList<Task*> tasks = AppContext::getTaskScheduler()->getTopLevelTasks();
    if(tasks.size() > 0){
        Task* topLevelTask = tasks.at(0);
        task = topLevelTask->getTaskName();

        QList<Task*> subtasks = topLevelTask->getSubtasks();
        if(subtasks.size() > 0){
            task = subtasks.at(0)->getTaskName();
        }
    }else{
        task = "";
    }
}

void SplashScreenWidget::getDots(){
    if(dots_number >= 3){
        dots_number = 0;
    }else{
        dots_number++;
    }
}

void SplashScreenWidget::timerEvent(QTimerEvent *e){
    getTask();
    getDots();
    drawInfo();

    update();
    QObject::timerEvent(e);
}

void SplashScreenWidget::paintEvent(QPaintEvent *e){
    QWidget::paintEvent(e);

    QPainter p(this);
    p.drawImage(0, 0, image2);
}

void SplashScreenWidget::drawInfo(){
    image2 = image1;

    QRect rectVersion = QRect(17, 268, width()-width()/3, TEXT_HEIGHT_PT);

    QPainter p(&image2);

    QFont font = p.font();
    font.setPixelSize( rectVersion.height() );
    font.setFamily("Heiti");
    p.setFont( font );
    p.setPen(QColor(0, 46, 59));
    QString text = "Version " + version + " is loading";
    for (int i = 0; i < dots_number; i++) {
        text.append(".");
    }
    p.drawText(rectVersion, text);

    if(!task.isEmpty()){
        QRect rect2 = QRect(17, 268 + TEXT_HEIGHT_PT + TEXT_PADDING_PT, width()-width()/3, TEXT_HEIGHT_PT);
        QString text2 = task;
        p.drawText(rect2, text2);
    }
    p.end();
}
}
