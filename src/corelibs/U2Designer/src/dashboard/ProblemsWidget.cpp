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

#include <U2Core/U2SafePoints.h>

#include "ProblemsWidget.h"

namespace U2 {

ProblemsWidget::ProblemsWidget(const QWebElement &content, Dashboard *parent)
: TableWidget(content, parent)
{
    createTable();
    connect(dashboard->monitor(), SIGNAL(si_newProblem(const Monitor::Problem &)), SLOT(sl_newProblem(const Monitor::Problem &)));
}

void ProblemsWidget::sl_newProblem(const Problem &info) {
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, );
    if (rows.contains(id(info))) {
        updateRow(id(info), createMultiRow(info));
    } else {
        addRow(id(info), createRow(info));
    }
}

QStringList ProblemsWidget::createRow(const Problem &info, bool multi) const {
    QStringList result;
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, result);

    QString prefix;
    if (multi) {
        int count = 0;
        foreach (const Problem &p, m->getProblems()) {
            if (p == info) {
                count++;
            }
        }
        prefix = QString("(%1) ").arg(count);
    }
    result << wrapLongText(m->actorName(info.actor));
    result << wrapLongText(prefix + info.message);
    return result;
}

QStringList ProblemsWidget::createMultiRow(const Problem &info) const {
    return createRow(info, true);
}

QStringList ProblemsWidget::createRow(const Problem &info) const {
    return createRow(info, false);
}

QList<int> ProblemsWidget::widths() {
    return QList<int>() << 30 << 70;
}

QStringList ProblemsWidget::header() {
    return QStringList() << tr("Element") << tr("Message");
}

QList<QStringList> ProblemsWidget::data() {
    QList<QStringList> result;
    const WorkflowMonitor *m = dashboard->monitor();
    CHECK(NULL != m, result);
    foreach (const Problem &info, m->getProblems()) {
        QStringList row;
        row << id(info);
        row << createRow(info);
        result << row;
    }
    return result;
}

QString ProblemsWidget::id(const Problem &info) const {
    return info.actor + info.message;
}

} // U2
