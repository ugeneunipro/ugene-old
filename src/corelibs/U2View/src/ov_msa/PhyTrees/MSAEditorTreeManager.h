/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MSAEDITOR_TREE_MANAGER_H_
#define _U2_MSAEDITOR_TREE_MANAGER_H_

#include <QtCore/QObject>
#include <U2Algorithm/CreatePhyTreeSettings.h>

namespace U2 {

class MSAEditor;
class MAlignmentObject;
class PhyTreeGeneratorLauncherTask;
class Task;
class GObjectViewWindow;


class MSAEditorTreeManager : public QObject {
    Q_OBJECT
public:
    MSAEditorTreeManager(MSAEditor* _editor);

    void loadRelatedTrees();

    void addTreeToMSA();
    void deleteTree();
    void refreshTree();
    void changeTreeSettings();

    void buildTreeWithDialog();
private slots:
    void sl_openTree();
    void sl_openTreeTaskFinished(Task* t);
    void sl_onWindowClosed(GObjectViewWindow* viewWindow);

private:
    MSAEditor*                    editor;
    MAlignmentObject*             msaObject;
    CreatePhyTreeSettings         settings;
    PhyTreeGeneratorLauncherTask* treeGeneratorTask; 
};

}//namespace
#endif
