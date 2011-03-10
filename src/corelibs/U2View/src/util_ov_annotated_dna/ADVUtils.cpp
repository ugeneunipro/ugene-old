#include "ADVUtils.h"
#include "AnnotatedDNAView.h"
#include "ADVSingleSequenceWidget.h"
#include "ADVSequenceObjectContext.h"


namespace U2 {

ADVGlobalAction::ADVGlobalAction(AnnotatedDNAView* v, const QIcon& icon, const QString& text, int ps, ADVGlobalActionFlags fl) 
: GObjectViewAction(v, v, text), pos(ps), flags(fl)
{
    setIcon(icon);
    connect(v, SIGNAL(si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*)), SLOT(sl_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*)));
    updateState();
    v->addADVAction(this);
}

void ADVGlobalAction::sl_focusChanged(ADVSequenceWidget* fromW, ADVSequenceWidget* toW) {
    Q_UNUSED(fromW); Q_UNUSED(toW);
    updateState();
}

void ADVGlobalAction::updateState() {
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(getObjectView());
    ADVSequenceWidget* w = av->getSequenceWidgetInFocus();
    bool enabled = w!=NULL;
    if (enabled && flags.testFlag(ADVGlobalActionFlag_SingleSequenceOnly) && qobject_cast<ADVSingleSequenceWidget*>(w) == NULL) {
        enabled = false;
    }
    if (enabled && !alphabetFilter.isEmpty()) {
        DNAAlphabetType t = w->getActiveSequenceContext()->getAlphabet()->getType();
        enabled = alphabetFilter.contains(t);
    }
    setEnabled(enabled);
}

} //namespace
