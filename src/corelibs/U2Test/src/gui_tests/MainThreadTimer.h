/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MAIN_THREAD_TIMER_H_
#define _U2_MAIN_THREAD_TIMER_H_

#include <QTimer>
#include <QMutex>

namespace U2 {

class MainThreadTimer : public QObject {
    Q_OBJECT
public:
    MainThreadTimer(int interval, QObject *parent);

    qint64 getCounter() const;

private slots:
    void sl_timerTick();

private:
    QTimer timer;
    mutable QMutex guard;
    qint64 counter;
};

}   // namespace U2

#endif // _U2_MAIN_THREAD_TIMER_H_
