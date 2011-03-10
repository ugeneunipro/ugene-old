#ifndef _U2_TREE_VIEWER_TASKS_H_
#define _U2_TREE_VIEWER_TASKS_H_

#include <U2Core/GObjectReference.h>
#include <U2Gui/ObjectViewTasks.h>

namespace U2 {

class PhyTreeObject;
class UnloadedObject;
class TreeViewer;
class CreateRectangularBranchesTask;

class OpenTreeViewerTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenTreeViewerTask(PhyTreeObject* obj);
    OpenTreeViewerTask(UnloadedObject* obj);
    OpenTreeViewerTask(Document* doc);

    virtual void open();

private:
    QPointer<PhyTreeObject>     phyObject;
    GObjectReference            unloadedReference;
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
    QString viewName;
    QPointer<PhyTreeObject> phyObj;
    CreateRectangularBranchesTask* subTask;
    QVariantMap stateData;
public:
    CreateTreeViewerTask(const QString& name, const QPointer<PhyTreeObject>& obj, const QVariantMap& stateData);
    virtual void prepare();
    virtual ReportResult report();
};

} // namespace

#endif
