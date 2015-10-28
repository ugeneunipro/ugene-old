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


#include "GTTestsQuerryDesigner.h"

#include "drivers/GTMouseDriver.h"
#include "drivers/GTKeyboardDriver.h"
#include "api/GTKeyboardUtils.h"
#include "api/GTWidget.h"
#include "api/GTFileDialog.h"
#include "api/GTMenu.h"
#include "GTGlobals.h"
#include "api/GTTreeWidget.h"
#include "runnables/qt/PopupChooser.h"
#include "GTUtilsQueryDesigner.h"

#include <U2Gui/ToolsMenu.h>

#include <QGraphicsItem>
#include <QGraphicsView>

namespace U2 {


namespace GUITest_common_scenarios_querry_designer {
using namespace HI;

void test1(U2OpStatus &os, QString s="") {
//Bug: QD: Crash while resizing and deleting elements (0002402)
//1. Open Query Designer
    GTUtilsQueryDesigner::openQueryDesigner(os);

    QString array[] = {"CDD", "Base Content","HMM2","HMM3","ORF"};
    QPoint p;
//2. Add any algorithm to the scheme
    for(int i=0;i<5; i++){
//3. Reduce any elements size by dragging its right corner as far to the left as possible
//  (or a bit less than as far as possible)
    if (s=="arr"){
        GTUtilsQueryDesigner::addAlgorithm(os,array[i]);

        GTMouseDriver::moveTo(os,GTUtilsQueryDesigner::getItemCenter(os, array[i]));
        p = GTMouseDriver::getMousePosition();
        p.setX(GTUtilsQueryDesigner::getItemRight(os, array[i]));
    }
    else{
        GTUtilsQueryDesigner::addAlgorithm(os,s);

        GTMouseDriver::moveTo(os,GTUtilsQueryDesigner::getItemCenter(os, s));
        p = GTMouseDriver::getMousePosition();
        p.setX(GTUtilsQueryDesigner::getItemRight(os, s));
    }
    GTMouseDriver::moveTo(os,p);

    GTMouseDriver::press(os);
    if (s=="arr"){
        p.setX(GTUtilsQueryDesigner::getItemLeft(os,array[i])+100);
    }
    else{
        p.setX(GTUtilsQueryDesigner::getItemLeft(os,s)+100);
    }
    GTMouseDriver::moveTo(os,p);

    GTMouseDriver::release(os);

//4. Select the resized element and press <Del>
    if (s=="arr"){
        GTMouseDriver::moveTo(os,GTUtilsQueryDesigner::getItemCenter(os, array[i]));
    }
    else{
        GTMouseDriver::moveTo(os,GTUtilsQueryDesigner::getItemCenter(os, s));
    }
    GTMouseDriver::click(os);
    GTKeyboardDriver::keyClick(os,GTKeyboardDriver::key["delete"]);
    GTGlobals::sleep(500);
    //check no elements on scene
    QGraphicsView* sceneView = qobject_cast<QGraphicsView*>(GTWidget::findWidget(os,"sceneView"));
    QList<QGraphicsItem *> items = sceneView->items();
    CHECK_SET_ERR(items.size() == 2, "Delete shortcut is not working");//2 - is equal empty scene
    }
//5. repeat from step 2 (do 5 iterations)
//Expected state: UGENE not crashes.
}
GUI_TEST_CLASS_DEFINITION(test_0001){
    test1(os, "Pattern");
}

GUI_TEST_CLASS_DEFINITION(test_0001_1){
    test1(os, "CDD");
}


GUI_TEST_CLASS_DEFINITION(test_0001_2){
    test1(os, "arr");
}
} // namespace GUITest_common_scenarios_annotations_edit
} // namespace U2
