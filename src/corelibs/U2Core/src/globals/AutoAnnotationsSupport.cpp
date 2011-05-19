/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/Settings.h>
#include <U2Core/RemoveAnnotationsTask.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/U2SafePoints.h>

#include "AutoAnnotationsSupport.h"

namespace U2 {

const QString AutoAnnotationObject::AUTO_ANNOTATION_HINT("auto-annotation object");

AutoAnnotationsUpdater::AutoAnnotationsUpdater( const QString& nm, const QString& gName )
: groupName(gName), name(nm)
{
    
    checkedByDefault = AppContext::getSettings()->getValue(AUTO_ANNOTATION_SETTINGS + groupName, false).toBool();
}

AutoAnnotationsUpdater::~AutoAnnotationsUpdater()
{
    AppContext::getSettings()->setValue(AUTO_ANNOTATION_SETTINGS + groupName, checkedByDefault);
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

AutoAnnotationsUpdater* AutoAnnotationsSupport::findUpdaterByName( const QString& name )
{
    foreach (AutoAnnotationsUpdater* updater, aaUpdaters) {
        if (updater->getName() == name) {
            return updater;
        }
    }

    return NULL;   
}

//////////////////////////////////////////////////////////////////////////

void AutoAnnotationsSupport::registerAutoAnnotationsUpdater( AutoAnnotationsUpdater* updater )
{
    aaUpdaters.append(updater);
}

void AutoAnnotationsSupport::unregisterAutoAnnotationsUpdater(AutoAnnotationsUpdater* updater){
    aaUpdaters.removeOne(updater);
}

AutoAnnotationsSupport::~AutoAnnotationsSupport()
{
    qDeleteAll(aaUpdaters);
}

bool AutoAnnotationsSupport::isAutoAnnotation( const AnnotationTableObject* obj )
{
    return obj->getGHintsMap().value(AutoAnnotationObject::AUTO_ANNOTATION_HINT).toBool();
}

bool AutoAnnotationsSupport::isAutoAnnotation( const GObject* obj )
{
    bool isAnnotationObject = obj->getGObjectType() == GObjectTypes::ANNOTATION_TABLE;
    bool hasAutoAnnotationHint = obj->getGHintsMap().value(AutoAnnotationObject::AUTO_ANNOTATION_HINT).toBool();
    return isAnnotationObject && hasAutoAnnotationHint;
}


//////////////////////////////////////////////////////////////////////////

AutoAnnotationObject::AutoAnnotationObject( DNASequenceObject* obj ) : dnaObj(obj)
{
    QVariantMap hints;
    hints.insert(AUTO_ANNOTATION_HINT, true);
    aobj = new AnnotationTableObject(AutoAnnotationsSupport::tr("Auto-annotations [%1 | %2]")
        .arg(obj->getDocument()->getName()).arg(obj->getSequenceName()), hints);
    aobj->addObjectRelation(dnaObj, GObjectRelationRole::SEQUENCE);
    aaSupport = AppContext::getAutoAnnotationsSupport();
}

AutoAnnotationObject::~AutoAnnotationObject()
{
    delete aobj;
}

void AutoAnnotationObject::update()
{
    QList<AutoAnnotationsUpdater*> aaUpdaters = aaSupport->getAutoAnnotationUpdaters();
    handleUpdate(aaUpdaters);
}

void AutoAnnotationObject::updateGroup( const QString& groupName )
{
    AutoAnnotationsUpdater* updater = aaSupport->findUpdaterByGroupName(groupName);
    if (updater != NULL) {
        QList<AutoAnnotationsUpdater*> updaters;
        updaters << updater;
        handleUpdate(updaters);
    }
}

void AutoAnnotationObject::handleUpdate( QList<AutoAnnotationsUpdater*> updaters )
{
    
    QList<Task*> subTasks;
    QStringList groupNames;
    
    foreach (AutoAnnotationsUpdater* updater, updaters) {
        
        // check constraints
        AutoAnnotationConstraints cns;
        cns.alphabet = dnaObj->getAlphabet();
        if (!updater->checkConstraints(cns)) {
            continue;
        }
        
        // cleanup 
        AnnotationGroup* root = aobj->getRootGroup();
        AnnotationGroup* sub = root->getSubgroup(updater->getGroupName(), false);
        if (sub != NULL) {
            Task* t = new RemoveAnnotationsTask(aobj, updater->getGroupName());
            subTasks.append(t);
        }

        // update
        if (enabledGroups.contains(updater->getGroupName())) {
            // create update tasks
            Task* t = updater->createAutoAnnotationsUpdateTask(this);
            if (t != NULL) {
                subTasks.append(t);
            }
        }
        
        groupNames.append(updater->getGroupName());
    }
    
    // envelope to unlock annotation object
    if (!subTasks.isEmpty()) {
        AutoAnnotationsUpdateTask* updateTask = new AutoAnnotationsUpdateTask(this, subTasks);
        AppContext::getTaskScheduler()->registerTopLevelTask(updateTask);
    }

}

void AutoAnnotationObject::setGroupEnabled( const QString& groupName, bool enabled )
{
    if (enabled) {
        enabledGroups.insert(groupName);
    } else {
        enabledGroups.remove(groupName);
    }
}

void AutoAnnotationObject::emitStateChange( bool started )
{
    if (started) {
        emit si_updateStarted();
    } else {
        emit si_updateFinshed();
    }
}



//////////////////////////////////////////////////////////////////////////

const QString AutoAnnotationsUpdateTask::NAME("Auto-annotations update task");

AutoAnnotationsUpdateTask::AutoAnnotationsUpdateTask( AutoAnnotationObject* aaObj, QList<Task*> updateTasks ) :
    Task(NAME, TaskFlags_NR_FOSCOE), aa(aaObj), lock(NULL), subTasks(updateTasks)
{
    setMaxParallelSubtasks(1);
}

void AutoAnnotationsUpdateTask::prepare()
{
    SAFE_POINT(aa != NULL, tr("Empty auto-annotation object"), );
    
    lock = new StateLock("Auto-annotations update", StateLockFlag_LiveLock);
    aa->getSeqObject()->lockState(lock);
    
    aa->emitStateChange(true);
    foreach(Task* subtask, subTasks) {
        addSubTask(subtask);
    }
}


Task::ReportResult AutoAnnotationsUpdateTask::report()
{
    if (aa != NULL ) {
        aa->emitStateChange(false);
    }

    if (lock != NULL) {
        aa->getSeqObject()->unlockState(lock);
        delete lock;
    }
    
    
    return ReportResult_Finished;
}




} //namespace
