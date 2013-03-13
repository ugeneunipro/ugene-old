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

#include "GTTestsWorkflowDesigner.h"

#include "api/GTMouseDriver.h"
#include "api/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "api/GTGlobals.h"
#include "api/GTTreeWidget.h"
#include "api/GTAction.h"
#include "runnables/qt/PopupChooser.h"
#include "GTUtilsWorkflowDesigner.h"
#include "GTUtilsApp.h"

#include <QGraphicsItem>
#include <U2Core/AppContext.h>
#include "../../workflow_designer/src/WorkflowViewItems.h"

namespace U2 {

//8 - text
//65536 - frame without ports
//65537 - frame with ports
//65538 - ports


namespace GUITest_common_scenarios_workflow_designer {
GUI_TEST_CLASS_DEFINITION(test_0001){
    QMenu* menu=GTMenu::showMainMenu(os, MWMENU_TOOLS);
    //GTMenu::clickMenuItem(os,)
    GTMenu::clickMenuItem(os, menu, QStringList() << "Workflow Designer");
    //GTFileDialog::openFile(os, testDir + "_common_data/workflow", "remoteDBReaderTest.uws");
    GTGlobals::sleep();
    //QMainWindow * = AppContext::getMainWindow()->getQMainWindow();
    GTUtilsWorkflowDesigner::addAlgorithm(os,"Create Alignment Example");

    //CHECK_SET_ERR(false, item->data(0,Qt::UserRole).value<QAction*>()->text());
    QGraphicsView* scene=qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));

    CHECK_SET_ERR(scene!=NULL,"scene is NULL");
    //QGraphicsScene* scene= GTWidget::findWidget(os,"splitter")->findChild<QGraphicsScene*>("scene");
        QList<QGraphicsItem*> list = scene->items();
        //QList<QLineF*> arrows = scene->findChildren<QLineF*>();
        QString actStr;
        foreach(QGraphicsItem* act, list){
            //QGraphicsObject *itObj = act->toGraphicsObject();
            //QObject* item = qobject_cast<QObject*>(itObj);
            //act->t
            //WorkflowPortItem* item1 = qobject_cast<WorkflowProcessItem*>(item);
            //QTextItem* item = (QTextItem*)(act);
            //if(act->type()==65538){
            //    act->moveBy(0,100);
            //    GTGlobals::sleep(1500);
            //}
            //QString s = act->metaObject()->className();
            //actStr.append( "\nitem  " + QString::number(act->type()));
            //}
        }
        //CHECK_SET_ERR(false, actStr);
}

} // namespace GUITest_common_scenarios_annotations_edit
} // namespace U2
