#ifndef _U2_ETS_PROJECT_VIEW_ITEMS_CONTOLLER_H
#define _U2_ETS_PROJECT_VIEW_ITEMS_CONTOLLER_H

#include <U2Core/global.h>

#include <QtGui/QAction>
#include <QtGui/QMenu>

#include "utils/ExternalToolSupportAction.h"

namespace U2 {

class ETSProjectViewItemsContoller : public QObject {
    Q_OBJECT
public:
    ETSProjectViewItemsContoller(QObject* p);
private slots:
    void sl_addToProjectViewMenu(QMenu&);

    void sl_runFormatDBOnSelection();
private:
    ExternalToolSupprotAction* formatDBOnSelectionAction;
    ExternalToolSupprotAction* makeBLASTDBOnSelectionAction;
};
}//namespace
#endif // _U2_ETS_PROJECT_VIEW_ITEMS_CONTOLLER_H
