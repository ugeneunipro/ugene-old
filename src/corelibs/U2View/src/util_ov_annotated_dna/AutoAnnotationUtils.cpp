#include <U2Core/AppContext.h>
#include <U2Core/AutoAnnotationsSupport.h>
#include <U2Gui/MainWindow.h>
#include <U2Gui/ObjectViewModel.h>

#include "ADVSequenceObjectContext.h"
#include "AutoAnnotationUtils.h"


namespace U2 {

AutoAnnotationsADVAction::AutoAnnotationsADVAction(ADVSequenceWidget* v) : ADVSequenceWidgetAction("aa", tr("Automatic annotation highlighting"))
{
    seqWidget = v;
    addToBar = true;

    menu = new QMenu();
    setIcon(QIcon(":core/images/predefined_annotation_groups.png"));
    setMenu(menu);
    
    updateMenu();

    connect( AppContext::getAutoAnnotationsSupport(), 
        SIGNAL(si_updateAutoAnnotationsGroupRequired(const QString&)), SLOT(sl_onAutoAnnotationsUpdate(const QString&)) );
}

void AutoAnnotationsADVAction::sl_windowActivated( MWMDIWindow* w )
{
    GObjectViewWindow* gow = qobject_cast<GObjectViewWindow*>(w);
    if (gow == NULL) {
        return;
    }
    
    if ((GObjectView*)seqWidget->getAnnotatedDNAView() == gow->getObjectView()) {
        updateMenu();   
    }

}

void AutoAnnotationsADVAction::updateMenu()
{
    menu->clear();
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
        toggleAction->setChecked(updater->isEnabled());
        connect(toggleAction, SIGNAL(triggered(bool)), updater, SLOT(toggle(bool)) );
        menu->addAction(toggleAction);
    }
    menu->update();
}

void AutoAnnotationsADVAction::sl_onAutoAnnotationsUpdate( const QString& groupName )
{
    AutoAnnotationsUpdater* updater = AppContext::getAutoAnnotationsSupport()->findUpdaterByGroupName(groupName);
    const QString& actionText = updater->getName();

    QList<QAction*> actions = menu->actions();
    foreach(QAction* action, actions) {
        if (action->text() == actionText) {
            action->setChecked(updater->isEnabled());
        }
    }
}



} //namespace
