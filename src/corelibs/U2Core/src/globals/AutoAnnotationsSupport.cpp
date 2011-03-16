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

#include "AutoAnnotationsSupport.h"

namespace U2 {


AutoAnnotationsUpdater::AutoAnnotationsUpdater( const QString& nm, const QString& gName )
: groupName(gName), name(nm)
{
    
    checkedByDefault = AppContext::getSettings()->getValue(AUTO_ANNOTATION_SETTINGS + groupName, true).toBool();
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
        update(updater);  
    }
}

void AutoAnnotationObject::updateGroup( const QString& groupName )
{
    AutoAnnotationsUpdater* updater = aaSupport->findUpdaterByGroupName(groupName);
    if (updater != NULL) {
        update(updater);
    }
}

void AutoAnnotationObject::update( AutoAnnotationsUpdater* updater )
{
    // cleanup 
    AnnotationGroup* root = aobj->getRootGroup();
    AnnotationGroup* sub = root->getSubgroup(updater->getGroupName(), false);
    if (sub != NULL) {
        unlock();
        root->removeSubgroup(sub);
        lock();
    }

    if (enabledGroups.contains(updater->getGroupName())) {
        // create update tasks
        Task* t = updater->createAutoAnnotationsUpdateTask(this);
        if (t == NULL) {
            return;
        }

        // envelope to unlock annotation object
        AutoAnnotationsUpdateTask* updateTask = new AutoAnnotationsUpdateTask(this, t);

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




} //namespace
