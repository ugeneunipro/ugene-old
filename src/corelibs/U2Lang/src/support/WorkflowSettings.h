/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_WORKFLOW_SETTINGS_H_
#define _U2_WORKFLOW_SETTINGS_H_

#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtGui/QFont>

#include <U2Core/global.h>

namespace U2 {

class Watcher : public QObject {
    Q_OBJECT
signals:
    void changed();
    friend class WorkflowSettings;
};

#define SETTINGS QString("workflowview/")

class U2LANG_EXPORT WorkflowSettings {
public:
    static bool showGrid();
    static void setShowGrid(bool v);

    static bool snap2Grid();
    static void setSnap2Grid(bool v);

    static bool monitorRun();
    static void setMonitorRun(bool v);

    /*static bool failFast();
    static void setFailFast(bool v);*/

    static QString defaultStyle();
    static void setDefaultStyle(const QString&);

    static QFont defaultFont();
    static void setDefaultFont(const QFont&);

    static const QString getUserDirectory();
    static void setUserDirectory(const QString &dir);

    static QColor getBGColor();
    static void setBGColor(const QColor &color);
    
    static int getRunMode();
    static void setRunMode(int md);
    
    static bool getScriptingMode();
    static void setScriptingMode(bool md);
    
    static bool runInSeparateProcess();
    static void setRunInSeparateProcess(bool m);
    static bool hasRunInSeparateProcess();
    
    static void setCmdlineUgenePath(const QString & path);
    static QString getCmdlineUgenePath();

    static void setExternalToolDirectory(const QString &newDir);
    static const QString getExternalToolDirectory();
    
    static Watcher * const watcher;
};

}//namespace

#endif
