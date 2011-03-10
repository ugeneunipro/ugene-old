#ifndef _U2_GUI_UTILS_H_
#define _U2_GUI_UTILS_H_

#include <U2Core/global.h>

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QTreeWidgetItem>
#include <QtCore/QList>


namespace U2 {

class U2GUI_EXPORT GUIUtils : public QObject {
    Q_OBJECT
public:
    
    static QAction* findAction(const QList<QAction*>& actions, const QString& name);
    
    static QAction* findActionAfter(const QList<QAction*>& actions, const QString& name);

    static QMenu* findSubMenu(QMenu* m, const QString& name);

    static void disableEmptySubmenus(QMenu* m);

    static QIcon  createSquareIcon(const QColor& c, int size);
    static QIcon  createRoundIcon(const QColor& c, int size);
    
    
    // Sets 'muted' look and feel. The item looks like disabled but still active and can be selected
    static void setMutedLnF(QTreeWidgetItem* item, bool disabled, bool recursive = false);
    static bool isMutedLnF(QTreeWidgetItem* item);
    

    static bool runWebBrowser(const QString& url);
};

} //namespace

#endif
