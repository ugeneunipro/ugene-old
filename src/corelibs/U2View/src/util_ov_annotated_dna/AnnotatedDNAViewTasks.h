#ifndef _U2_ANNOTATED_DNA_VIEW_TASKS_H_
#define _U2_ANNOTATED_DNA_VIEW_TASKS_H_

#include <U2Gui/ObjectViewTasks.h>
#include <U2Core/GObjectReference.h>

namespace U2 {

class DNASequenceObject;
class AnnotatedDNAView;

class OpenAnnotatedDNAViewTask : public ObjectViewTask {
    Q_OBJECT
public:
    //opens a single view for all sequence object in the list of sequence objects related to the objects in the list
    OpenAnnotatedDNAViewTask(const QList<GObject*>& objects);

    virtual void open();

private:
    QList<GObjectReference> sequenceObjectRefs;
};

class OpenSavedAnnotatedDNAViewTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenSavedAnnotatedDNAViewTask(const QString& viewName, const QVariantMap& stateData);
    virtual void open();

};


class UpdateAnnotatedDNAViewTask : public ObjectViewTask {
public:
    UpdateAnnotatedDNAViewTask(AnnotatedDNAView* v, const QString& stateName, const QVariantMap& stateData);

    virtual void update();
};


} // namespace

#endif
