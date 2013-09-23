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

#include "ExternalToolsWidget.h"

#include <U2Core/U2SafePoints.h>

#include <U2Lang/Dataset.h>
#include <U2Lang/URLAttribute.h>
#include <U2Lang/URLContainer.h>
#include <U2Lang/WorkflowUtils.h>


namespace U2 {

    const QString ExternalToolsWidget::LINE_BREAK("break_line");
    const QString ExternalToolsWidget::SINGLE_QUOTE("s_quote");
    const QString ExternalToolsWidget::BACK_SLASH("b_slash");

    ExternalToolsWidget::ExternalToolsWidget(const QWebElement &container, Dashboard *parent)
        : DashboardWidget(container, parent)
    {
        const WorkflowMonitor *workflowMonitor = dashboard->monitor();
        SAFE_POINT(NULL != workflowMonitor, "NULL workflow monitor!", );

        bool res = connect(workflowMonitor, SIGNAL(si_logChanged(QString, QString, int , int, QString)), SLOT(sl_onLogChanged(QString, QString, int , int, QString)));

        const QMap<QString, Monitor::WorkerLogInfo> &workersLogInfo = workflowMonitor->getWorkersLog();
        ExternalToolsWidget::container.evaluateJavaScript("lwInitConteiner(this, 'params_tab_id_0')");
    }

    void ExternalToolsWidget::sl_onLogChanged(QString toolName, QString actorName, int runNumber, int logType, QString lastLine) {
        addInfoToWidget(toolName, actorName, runNumber, logType, lastLine);
    }

    void ExternalToolsWidget::addInfoToWidget(QString toolName, QString actorName, int runNumber, int logType, QString lastLine) {
        int i = 0;

        QString tabId = "log_tab_id_" + actorName;

        QString runId = toolName + " run " + QString::number(runNumber);
        QString lastPartOfLog = lastLine;

        lastPartOfLog.replace(QRegExp("\\n"), LINE_BREAK);
        lastPartOfLog.replace(QRegExp("\\r"), "");
        lastPartOfLog.replace("'", SINGLE_QUOTE);
        lastPartOfLog.replace('\\', BACK_SLASH);

        QString addLogFunc = "lwAddTreeNode";
        addLogFunc += "('" + runId + "', ";
        addLogFunc += "'" + actorName + "', ";
        addLogFunc += "'" + tabId + "', ";

        switch(logType) {
            case ERROR_LOG: 
                addLogFunc += "'" + lastPartOfLog + "', ";
                addLogFunc += "'error')";
                container.evaluateJavaScript(addLogFunc);
                break;
            case OUTPUT_LOG:
                addLogFunc += "'" + lastPartOfLog + "', ";
                addLogFunc += "'output')";
                container.evaluateJavaScript(addLogFunc);
                break;
            case PROGRAM_PATH:
                addLogFunc += "'" + lastPartOfLog + "', ";
                addLogFunc += "'program')";
                container.evaluateJavaScript(addLogFunc);
                break;
            case ARGUMENTS:
                addLogFunc += "'" + lastPartOfLog + "', ";
                addLogFunc += "'arguments')";
                container.evaluateJavaScript(addLogFunc);
                break;
        }
    }
} // namespace U2