#ifndef _U2_MSA_EDITOR_TASKS_H_
#define _U2_MSA_EDITOR_TASKS_H_

#include <U2Core/GObjectReference.h>
#include <U2Gui/ObjectViewTasks.h>

namespace U2 {

class MAlignmentObject;
class UnloadedObject;
class MSAEditor;

class OpenMSAEditorTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenMSAEditorTask(MAlignmentObject* obj);
    OpenMSAEditorTask(UnloadedObject* obj);
    OpenMSAEditorTask(Document* doc);

    virtual void open();

private:
    QPointer<MAlignmentObject>  msaObject;
    GObjectReference            unloadedReference;
};

class OpenSavedMSAEditorTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenSavedMSAEditorTask(const QString& viewName, const QVariantMap& stateData);
    virtual void open();

    static void updateRanges(const QVariantMap& stateData, MSAEditor* ctx);
    static void addAnnotations(const QVariantMap& stateData, MSAEditor* ctx);
};


class UpdateMSAEditorTask : public ObjectViewTask {
public:
    UpdateMSAEditorTask(GObjectView* v, const QString& stateName, const QVariantMap& stateData);

    virtual void update();
};


} // namespace

#endif
