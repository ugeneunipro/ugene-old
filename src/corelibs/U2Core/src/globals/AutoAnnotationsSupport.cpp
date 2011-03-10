#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/Settings.h>

#include "AutoAnnotationsSupport.h"

namespace U2 {

void AutoAnnotationsSupport::registerAutoAnnotationsUpdater( AutoAnnotationsUpdater* updater )
{
    aaUpdaters.append(updater);
}


QList<AutoAnnotationsUpdater*> AutoAnnotationsSupport::getAutoAnnotationUpdaters()
{
    return aaUpdaters;
}

AutoAnnotationsUpdater* AutoAnnotationsSupport::findUpdaterByGroupName( const QString& groupName )
{
    foreach (AutoAnnotationsUpdater* updater, aaUpdaters) {
        if (updater->getGroupName() == groupName) {
            return updater;
        }
    }
    
    return NULL;
}

void AutoAnnotationsSupport::updateAnnotationsByGroup( const QString& groupName )
{
    emit si_updateAutoAnnotationsGroupRequired(groupName);
}

AutoAnnotationsSupport::~AutoAnnotationsSupport()
{
    qDeleteAll(aaUpdaters);
}

//////////////////////////////////////////////////////////////////////////

AutoAnnotationObject::AutoAnnotationObject( DNASequenceObject* obj ) : dnaObj(obj)
{
    aobj = new AnnotationTableObject(AutoAnnotationsSupport::tr("Auto-annotations [%1 | %2]").arg(obj->getDocument()->getName()).arg(obj->getSequenceName()));
    aaSupport = AppContext::getAutoAnnotationsSupport();
    stateLock = new StateLock("Auto-annotation objects can not be modified");
    connect(aaSupport, SIGNAL(si_updateAutoAnnotationsGroupRequired(const QString&) ), SLOT(updateGroup(const QString&))  );
    lock();
}

AutoAnnotationObject::~AutoAnnotationObject()
{
    unlock();
    delete aobj;
    delete stateLock;
}

void AutoAnnotationObject::lock() {
    if (!aobj->isStateLocked()) {
        aobj->lockState(stateLock);
    }
}

void AutoAnnotationObject::unlock() {
    if (aobj->isStateLocked()) {
        aobj->unlockState(stateLock);
    }
}

void AutoAnnotationObject::update()
{
    QList<AutoAnnotationsUpdater*> aaUpdaters = aaSupport->getAutoAnnotationUpdaters();
    foreach( AutoAnnotationsUpdater* updater, aaUpdaters) {
        handleUpdate(updater);  
    }
}

void AutoAnnotationObject::updateGroup( const QString& groupName )
{
    AutoAnnotationsUpdater* updater = aaSupport->findUpdaterByGroupName(groupName);
    if (updater != NULL) {
        handleUpdate(updater);
    }
}

void AutoAnnotationObject::handleUpdate( AutoAnnotationsUpdater* updater )
{
    // cleanup 
    AnnotationGroup* root = aobj->getRootGroup();
    AnnotationGroup* sub = root->getSubgroup(updater->getGroupName(), false);
    if (sub != NULL) {
        unlock();
        root->removeSubgroup(sub);
        lock();
    }

    if (!updater->isEnabled()) {
        return;
    }

    // create update tasks
    Task* t = updater->createAutoAnnotationsUpdateTask(this);
    if (t == NULL) {
        return;
    }
    
    // envelope to unlock annotation object
    AutoAnnotationsUpdateTask* updateTask = new AutoAnnotationsUpdateTask(this, t);

    AppContext::getTaskScheduler()->registerTopLevelTask(updateTask);
}


//////////////////////////////////////////////////////////////////////////

AutoAnnotationsUpdateTask::AutoAnnotationsUpdateTask( AutoAnnotationObject* aaObj, Task* updateTask ) :
    Task("Auto-annotations update task", TaskFlags_NR_FOSCOE), aa(aaObj), subtask(updateTask)
{
       
}

void AutoAnnotationsUpdateTask::prepare()
{
    aa->unlock();
    addSubTask(subtask);
}

AutoAnnotationsUpdateTask::~AutoAnnotationsUpdateTask()
{
    aa->lock();
}


void AutoAnnotationsUpdater::toggle( bool enable )
{
    enabled = enable;
    AppContext::getAutoAnnotationsSupport()->updateAnnotationsByGroup(groupName);
}

AutoAnnotationsUpdater::AutoAnnotationsUpdater( const QString& nm, const QString& gName )
 : groupName(gName), name(nm)
{
    enabled = AppContext::getSettings()->getValue(AUTO_ANNOTATION_SETTINGS + groupName, true).toBool();
}

AutoAnnotationsUpdater::~AutoAnnotationsUpdater()
{
    AppContext::getSettings()->setValue(AUTO_ANNOTATION_SETTINGS + groupName, enabled);
}

} //namespace
