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
    
    static Watcher * const watcher;
};

}//namespace

#endif
