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

#include "ParametersWidget.h"

#include <U2Core/U2SafePoints.h>


namespace U2 {

ParametersWidget::ParametersWidget(const QWebElement &container, Dashboard *parent)
    : DashboardWidget(container, parent)
{
    const WorkflowMonitor *workflowMonitor = dashboard->monitor();
    SAFE_POINT(NULL != workflowMonitor, "NULL workflow monitor!", );

    const QList<WorkerParamsInfo> &workersParamsInfo = workflowMonitor->getWorkersParameters();
    createWidget(workersParamsInfo);
}

void ParametersWidget::createWidget(const QList<WorkerParamsInfo> &workersParamsInfo) {
    int i = 0;
    foreach (const WorkerParamsInfo &info, workersParamsInfo) {
        // Create the tab
        QString tabId = "params_tab_id_" + QString::number(i);
        QString createTabFunc;
        if (0 == i) {
            createTabFunc = "pwInitAndActiveTab";
        }
        else {
            createTabFunc = "pwAddTab";
        }
        createTabFunc += "(this, '" + info.workerName + "', '" + tabId + "')";

        container.evaluateJavaScript(createTabFunc);

        // Add the parameters
        foreach (QString paramName, info.paramsWithValues.keys()) {
            QString createParamFunc = "pwAddParameter";
            createParamFunc += "('" + tabId + "', ";
            createParamFunc += "'" + paramName + "', ";
            createParamFunc += "'" + info.paramsWithValues.value(paramName) + "')";

            container.evaluateJavaScript(createParamFunc);
        }

        // Increase the iterator
        i++;
    }
}


} // namespace U2