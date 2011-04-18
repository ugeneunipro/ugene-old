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
#include <U2Core/AutoAnnotationsSupport.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/ObjectViewModel.h>

#include "ADVSequenceObjectContext.h"
#include "AutoAnnotationUtils.h"


namespace U2 {

const QString AutoAnnotationsADVAction::ACTION_NAME("AutoAnnotationUpdateAction");

#define AUTO_ANNOTATION_GROUP_NAME "AutoAnnotatationGroupName"

AutoAnnotationsADVAction::AutoAnnotationsADVAction(ADVSequenceWidget* v, AutoAnnotationObject* obj) 
: ADVSequenceWidgetAction(ACTION_NAME, tr("Automatic Annotations Highlighting")), aaObj(obj)
{
    seqWidget = v;
    addToBar = true;

    menu = new QMenu();
    setIcon(QIcon(":core/images/predefined_annotation_groups.png"));
    setMenu(menu);
    
    updateMenu();
        
    connect(aaObj->getAnnotationObject(), SIGNAL(si_lockedStateChanged()), SLOT(sl_autoAnnotationLockStateChanged()));
    
    aaObj->update();
}


void AutoAnnotationsADVAction::updateMenu()
{
    AutoAnnotationConstraints constraints;
    if (seqWidget->getSequenceContexts().count() > 0) {
        constraints.alphabet = seqWidget->getSequenceContexts().first()->getAlphabet();
    }
    
    QList<AutoAnnotationsUpdater*> updaters = AppContext::getAutoAnnotationsSupport()->getAutoAnnotationUpdaters();
    if (updaters.count() == 0 ) {
        setEnabled(false);
        return;
    }
    foreach (AutoAnnotationsUpdater* updater, updaters) {
        QAction* toggleAction = new QAction(updater->getName(), this);
        toggleAction->setProperty(AUTO_ANNOTATION_GROUP_NAME, updater->getGroupName());
        bool enabled = updater->checkConstraints(constraints);
        toggleAction->setEnabled(enabled);
        toggleAction->setCheckable(true);
        bool checked = updater->isCheckedByDefault();
        toggleAction->setChecked(checked);
        aaObj->setGroupEnabled(updater->getGroupName(), checked);
        connect( toggleAction, SIGNAL(toggled(bool)), SLOT(sl_toggle(bool)) );
        menu->addAction(toggleAction);
    }
    
    menu->update();
}

void AutoAnnotationsADVAction::sl_toggle( bool toggled )
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action == NULL) {
        return;
    }
    AutoAnnotationsUpdater* updater = AppContext::getAutoAnnotationsSupport()->findUpdaterByName(action->text());
    if (updater != NULL) {
        QString groupName = updater->getGroupName();
        aaObj->setGroupEnabled(groupName, toggled);
        aaObj->updateGroup(groupName);
        updater->setCheckedByDefault(toggled);
    }
}

AutoAnnotationsADVAction::~AutoAnnotationsADVAction()
{
    menu->clear();
    delete menu;
    menu = NULL;
}

QList<QAction*> AutoAnnotationsADVAction::getToggleActions()
{
    return menu->actions();
}

void AutoAnnotationsADVAction::sl_autoAnnotationLockStateChanged()
{
    if (aaObj->isLocked() ) {
        setEnabled(true);
    } else {
        setEnabled(false);
    }
}

QAction* AutoAnnotationsADVAction::findToggleAction( const QString& groupName )
{
    QList<QAction*> toggleActions = menu->actions();
    foreach(QAction* tAction, toggleActions) {
        if (tAction->property(AUTO_ANNOTATION_GROUP_NAME) == groupName) {
            return tAction;
        }
    }
    return NULL;
}

QAction* AutoAnnotationUtils::findAutoAnnotationsToggleAction( ADVSequenceObjectContext* ctx, const QString& groupName )
{
    foreach(ADVSequenceWidget* w, ctx->getSequenceWidgets()) {
         ADVSequenceWidgetAction* advAction = w->getADVSequenceWidgetAction(AutoAnnotationsADVAction::ACTION_NAME);
         if (advAction == NULL) {
             continue;
         }
         AutoAnnotationsADVAction* aaAction = qobject_cast<AutoAnnotationsADVAction*> (advAction);
         assert(aaAction != NULL);
         QList<QAction*> toggleActions = aaAction->getToggleActions();
         foreach(QAction* tAction, toggleActions) {
             if (tAction->property(AUTO_ANNOTATION_GROUP_NAME) == groupName) {
                 return tAction;
             }
         }
    }

    return NULL;

}



void AutoAnnotationUtils::triggerAutoAnnotationsUpdate( ADVSequenceObjectContext* ctx, const QString& aaGroupName )
{
    AutoAnnotationsADVAction* aaAction = findAutoAnnotationADVAction( ctx );
    
    assert(aaAction != NULL);
    if (aaAction) {
        
        QAction* updateAction = aaAction->findToggleAction(aaGroupName);
        assert (updateAction != NULL);
        
        if (!updateAction) {
            return;
        }

        if (!updateAction->isChecked()) {
            updateAction->trigger();
        } else {
            AutoAnnotationsUpdater* updater = AppContext::getAutoAnnotationsSupport()->findUpdaterByGroupName(aaGroupName);
            if (updater != NULL) {
                aaAction->getAAObj()->updateGroup(aaGroupName);
            }
        }
    }
}

AutoAnnotationsADVAction* AutoAnnotationUtils::findAutoAnnotationADVAction( ADVSequenceObjectContext* ctx )
{
    foreach(ADVSequenceWidget* w, ctx->getSequenceWidgets()) {
        ADVSequenceWidgetAction* advAction = w->getADVSequenceWidgetAction(AutoAnnotationsADVAction::ACTION_NAME);
        if (advAction == NULL) {
            continue;
        } else {
            return qobject_cast<AutoAnnotationsADVAction*>(advAction);
        }
    }

    return NULL;
}

} //namespace
