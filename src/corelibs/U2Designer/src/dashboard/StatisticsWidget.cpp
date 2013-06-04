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

#include <QTimer>

#include "StatisticsWidget.h"

namespace U2 {

StatisticsWidget::StatisticsWidget(const QWebElement &container, Dashboard *parent)
: TableWidget(container, parent)
{
    useEmptyRows = false;
    createTable();
    fillTable();
    connect(dashboard->monitor(), SIGNAL(si_workerInfoChanged(const QString &, const Monitor::WorkerInfo &)),
        SLOT(sl_workerInfoChanged(const QString &, const Monitor::WorkerInfo &)));
    connect(dashboard->monitor(), SIGNAL(si_updateProducers()),
        SLOT(sl_update()));
}

void StatisticsWidget::sl_workerInfoChanged(const QString &actor, const WorkerInfo &info) {
    updateRow(actor, createRow(actor, info));
}

void StatisticsWidget::sl_update() {
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, );

    QMap<QString, WorkerInfo> infos = m->getWorkersInfo();
    foreach (const QString &actor, infos.keys()) {
        updateRow(actor, createRow(actor, infos[actor]));
    }
}

QList<int> StatisticsWidget::widths() {
    return QList<int>() << 40 << 30 << 30;
}

QStringList StatisticsWidget::header() {
    return QStringList() << tr("Element") << tr("Elapsed time") << tr("Produced data");
}

inline static QString timeStr(qint64 timeMks) {
    return QTime().addMSecs(timeMks/1000).toString("hh:mm:ss");
}

QList<QStringList> StatisticsWidget::data() {
    QList<QStringList> result;
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, result);
    QMap<QString, WorkerInfo> infos = m->getWorkersInfo();
    foreach (const QString &actorId, infos.keys()) {
        WorkerInfo &info = infos[actorId];
        QStringList row;
        row << actorId; // id
        row << createRow(actorId, info);
        result << row;
    }
    return result;
}

QStringList StatisticsWidget::createRow(const QString &actor, const WorkerInfo &info) {
    QStringList result;
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, result);

    result << wrapLongText(m->actorName(actor));
    result << timeStr(info.timeMks);
    result << QString::number(m->getDataProduced(actor));
    return result;
}

} // U2
