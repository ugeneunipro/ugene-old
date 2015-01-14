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

#ifndef _U2_LOG_WIDGET_H_
#define _U2_LOG_WIDGET_H_

#include "Dashboard.h"

namespace U2 {

using namespace Workflow::Monitor;

class ExternalToolsWidget;

class ExternalToolsWidgetController : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY(ExternalToolsWidgetController)
public:
    ExternalToolsWidgetController();
    ~ExternalToolsWidgetController() { delete timer; }

    ExternalToolsWidget* getWidget(const QWebElement &container, Dashboard *parent);
    LogEntry getEntry(int index) const;
    QList<LogEntry> getLog() const { return log; }
    int getLogSize() const { return log.count(); }

signals:
    void si_update();

public slots:
    void sl_onLogChanged(U2::Workflow::Monitor::LogEntry entry);

private slots:
    void sl_timerShouts();

private:
    ExternalToolsWidget* toolsWidget;
    QList<LogEntry> log;
    QTimer *timer;
};

class ExternalToolsWidget : public DashboardWidget {
    Q_OBJECT
public:
    ExternalToolsWidget(const QWebElement &container, Dashboard *parent, const ExternalToolsWidgetController* ctrl);

public slots:
    void sl_onLogUpdate();

private:
    void addInfoToWidget(const LogEntry &entry);
    bool isSameNode(const LogEntry& prev, const LogEntry& cur) const;

    int lastEntryIndex;
    const ExternalToolsWidgetController* ctrl;

    static const QString LINE_BREAK;
    static const QString BACK_SLASH;
    static const QString SINGLE_QUOTE;
};

} // namespace U2

#endif
