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

#ifndef _U2_DASHBOARD_H_
#define _U2_DASHBOARD_H_

#include <QWebElement>
#include <QWebView>

#include <U2Core/U2SafePoints.h>

#include <U2Lang/WorkflowMonitor.h>

namespace U2 {
using namespace Workflow;

class U2DESIGNER_EXPORT Dashboard : public QWebView {
    Q_OBJECT
public:
    Dashboard(const WorkflowMonitor *monitor, QWidget *parent);

    const WorkflowMonitor * monitor();
    QWebElement getDocument();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *ev);

private slots:
    void sl_runStateChanged(bool paused);
    void sl_loaded(bool ok);
    void sl_addProblemsWidget();

private:
    const WorkflowMonitor *_monitor;
    QWebElement doc;
    bool initialized;

    enum DashboardTab {OverviewDashTab, InputDashTab, OutputDashTab};

private:
    void loadDocument();
    /** Returns the content area of the widget */
    QWebElement addWidget(const QString &title, DashboardTab dashTab, int cntNum = -1);

    /** Returns size of the QWebElement "name", it is searched inside "insideElt" only*/
    int containerSize(const QWebElement &insideElt, const QString &name);
};

class DashboardWidget : public QObject {
    Q_OBJECT
public:
    DashboardWidget(const QWebElement &container, Dashboard *parent);

protected:
    Dashboard *dashboard;
    QWebElement container;
};

class JavascriptAgent : public QObject {
    Q_OBJECT
public slots:
    void openUrl(const QString &url);
    void openByOS(const QString &url);
};

} // U2

#endif // _U2_DASHBOARD_H_
