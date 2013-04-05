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
#include <U2Core/PhyTree.h>

namespace U2 {

class MSAEditor;
class MAlignmentObject;
class PhyTreeGeneratorLauncherTask;
class Task;
class GObjectViewWindow;
class MSAEditorTreeViewer;
class Document;
class PhyTreeObject;

class MSAEditorTreeManager : public QObject {
    Q_OBJECT
public:
    MSAEditorTreeManager(MSAEditor* _editor);
    ~MSAEditorTreeManager(){}

    void loadRelatedTrees();

    void addTreeToMSA();

    void buildTreeWithDialog();

    void showAddTreeDialog();
private slots:
    void sl_openTree();
    void sl_openTreeTaskFinished(Task* t);
    void sl_onWindowClosed(GObjectViewWindow* viewWindow);
    void sl_treeRebuildingFinished();
    void sl_refreshTree(MSAEditorTreeViewer& treeViewer);
    void sl_onPhyTreeDocLoaded(Task*);

private:
    void buildTree(const CreatePhyTreeSettings& buildSettings);
    bool canRefreshTree(MSAEditorTreeViewer* treeViewer);
    void openTreeViewer(PhyTreeObject* treeObj);
    void loadTreeFromFile(const QString& treeFileName);

    MSAEditor*                    editor;
    MAlignmentObject*             msaObject;
    CreatePhyTreeSettings         settings;
    PhyTreeGeneratorLauncherTask* treeGeneratorTask; 
    MSAEditorTreeViewer*          refreshingTree;
    bool                          addExistingTree;
    PhyTree                       phyTree;
    Document                      *d;
};

}//namespace
#endif
