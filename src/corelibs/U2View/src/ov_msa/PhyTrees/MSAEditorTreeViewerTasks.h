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

#ifndef _U2_TREE_VIEWER_TASKS_H_
#define _U2_TREE_VIEWER_TASKS_H_

#include <U2Core/GObjectReference.h>
#include <U2Gui/ObjectViewTasks.h>
#include <QtGui/QSplitter>

namespace U2 {

class PhyTreeObject;
class UnloadedObject;
class TreeViewer;
class CreateRectangularBranchesTask;
class OpenTreeViewerTask;

class OpenTreeViewerTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenTreeViewerTask(PhyTreeObject* obj, const QObject* _parent = NULL, bool _createMDIWindow = true);
    OpenTreeViewerTask(UnloadedObject* obj, const QObject* _parent = NULL, bool _createMDIWindow = true);
    OpenTreeViewerTask(Document* doc, const QObject* _parent = NULL, bool _createMDIWindow = true);
    virtual ~OpenTreeViewerTask();

    virtual void open();

    static void updateTitle(TreeViewer* tv);

private:
    QPointer<PhyTreeObject>     phyObject;
    GObjectReference            unloadedReference;
    const QObject*              parent;
    bool                        createMDIWindow;
};

class OpenSavedTreeViewerTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenSavedTreeViewerTask(const QString& viewName, const QVariantMap& stateData);
    virtual void open();

    static void updateRanges(const QVariantMap& stateData, TreeViewer* ctx);
};


class UpdateTreeViewerTask : public ObjectViewTask {
public:
    UpdateTreeViewerTask(GObjectView* v, const QString& stateName, const QVariantMap& stateData);
    virtual void update();
};

class CreateTreeViewerTask: public Task {
    Q_OBJECT
    QString viewName;
    QPointer<PhyTreeObject> phyObj;
    CreateRectangularBranchesTask* subTask;
    QVariantMap stateData;
    TreeViewer* view;
public:
    CreateTreeViewerTask(const QString& name, const QPointer<PhyTreeObject>& obj, const QVariantMap& stateData);
    virtual void prepare();
    virtual ReportResult report();
    TreeViewer* getTreeViewer();
    const QVariantMap& getStateData();
};

class CreateMDITreeViewerTask: public Task {
    Q_OBJECT
public:
    CreateMDITreeViewerTask(const QString& name, const QPointer<PhyTreeObject>& obj, const QVariantMap& stateData);
    virtual void prepare();
    virtual ReportResult report();
private:
    QString viewName;
    CreateTreeViewerTask* subTask;
};

} // namespace

#endif
