#include <U2Core/AppContext.h>
#include <U2Core/AutoAnnotationsSupport.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/ObjectViewModel.h>

#include "ADVSequenceObjectContext.h"
#include "AutoAnnotationUtils.h"


namespace U2 {

AutoAnnotationsADVAction::AutoAnnotationsADVAction(ADVSequenceWidget* v, AutoAnnotationObject* obj) 
: ADVSequenceWidgetAction("AutoAnnotationUpdateAction", tr("Automatic annotation highlighting")), aaObj(obj)
{
    seqWidget = v;
    addToBar = true;

    menu = new QMenu();
    setIcon(QIcon(":core/images/predefined_annotation_groups.png"));
    setMenu(menu);
    
    updateMenu();

}


void AutoAnnotationsADVAction::updateMenu()
{
    AutoAnnotationConstraints constraints;
    if (seqWidget->getSequenceContexts().count() > 0) {
        constraints.alphabet = seqWidget->getSequenceContexts().first()->getAlphabet();
    }

    QList<AutoAnnotationsUpdater*> updaters = AppContext::getAutoAnnotationsSupport()->getAutoAnnotationUpdaters();
    foreach (AutoAnnotationsUpdater* updater, updaters) {
        QAction* toggleAction = new QAction(updater->getName(), this);
        bool enabled = updater->checkConstraints(constraints);
        toggleAction->setEnabled(enabled);
        toggleAction->setCheckable(true);
        bool checked = updater->isCheckedByDefault();
        toggleAction->setChecked(checked);
        aaObj->setGroupEnabled(updater->getGroupName(), checked);
        aaObj->update(updater);
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
        aaObj->update(updater);
    }
}


} //namespace
