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

ExternalToolsWidget* ExternalToolsWidgetController::getWidget(const QWebElement &container, Dashboard *parent) const {
    ExternalToolsWidget* widget = new ExternalToolsWidget(container, parent, this);
    connect(this, SIGNAL(si_infoAdded(int)), widget, SLOT(sl_onInfoChanged(int)));
    return widget;
}

LogEntry ExternalToolsWidgetController::getEntry(int index) const {
    SAFE_POINT(index >= 0 && index < log.count(), "Invalid index", LogEntry());
    return log.at(index);
}

void ExternalToolsWidgetController::sl_onLogChanged(U2::Workflow::Monitor::LogEntry entry) {
    log << entry;
    emit si_infoAdded(log.count() - 1);
}

const QString ExternalToolsWidget::LINE_BREAK("break_line");
const QString ExternalToolsWidget::SINGLE_QUOTE("s_quote");
const QString ExternalToolsWidget::BACK_SLASH("b_slash");

ExternalToolsWidget::ExternalToolsWidget(const QWebElement &_container,
                                         Dashboard *parent,
                                         const ExternalToolsWidgetController *_ctrl) :
    DashboardWidget(_container, parent),
    ctrl(_ctrl)
{
    SAFE_POINT(NULL != ctrl, "Controller is NULL", );

    const WorkflowMonitor *workflowMonitor = dashboard->monitor();
    SAFE_POINT(NULL != workflowMonitor, "NULL workflow monitor!", );

    container.evaluateJavaScript("lwInitConteiner(this, 'params_tab_id_0')");

    foreach (LogEntry entry, ctrl->getLog()) {
        addInfoToWidget(entry);
    }
}

void ExternalToolsWidget::sl_onInfoChanged(int index) {
    SAFE_POINT(sender() == ctrl, "Unexpected sender", );
    addInfoToWidget(ctrl->getEntry(index));
}

void ExternalToolsWidget::addInfoToWidget(const LogEntry &entry) {
    QString tabId = "log_tab_id_" + entry.actorName;

    QString runId = entry.toolName + " run " + QString::number(entry.runNumber);
    QString lastPartOfLog = entry.lastLine;

    lastPartOfLog.replace(QRegExp("\\n"), LINE_BREAK);
    lastPartOfLog.replace(QRegExp("\\r"), "");
    lastPartOfLog.replace("'", SINGLE_QUOTE);
    lastPartOfLog.replace('\\', BACK_SLASH);

    QString addLogFunc = "lwAddTreeNode";
    addLogFunc += "('" + runId + "', ";
    addLogFunc += "'" + entry.actorName + "', ";
    addLogFunc += "'" + tabId + "', ";

    switch(entry.logType) {
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
