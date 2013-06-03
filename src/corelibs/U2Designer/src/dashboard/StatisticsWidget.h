/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_STATISTICSWIDGET_H_
#define _U2_STATISTICSWIDGET_H_

#include "TableWidget.h"

namespace U2 {

using namespace Workflow::Monitor;

class StatisticsWidget : public TableWidget {
    Q_OBJECT
public:
    StatisticsWidget(const QWebElement &container, Dashboard *parent);

    virtual QList<int> widths();
    virtual QStringList header();
    virtual QList<QStringList> data();

private slots:
    void sl_workerInfoChanged(const QString &actor, const Monitor::WorkerInfo &info);
    void sl_update();

private:
    QStringList createRow(const QString &actor, const Monitor::WorkerInfo &info);
};

} // U2

#endif // _U2_STATISTICSWIDGET_H_
