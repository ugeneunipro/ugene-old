#include "ADVSequenceWidget.h"
#include "ADVSequenceObjectContext.h"
#include "AnnotatedDNAView.h"

namespace U2 {

ADVSequenceWidget::ADVSequenceWidget(AnnotatedDNAView* _ctx) :QWidget(_ctx->getScrolledWidget()), ctx(_ctx) {
}

void ADVSequenceWidget::addADVSequenceWidgetAction(ADVSequenceWidgetAction* action) {
    assert(!action->objectName().isEmpty());
    assert(getADVSequenceWidgetAction(action->objectName()) == NULL);
    action->setParent(this);
    action->seqWidget = this;
    wActions.append(action);
}

ADVSequenceWidgetAction* ADVSequenceWidget::getADVSequenceWidgetAction(const QString& objName) const {
    Q_UNUSED(objName);
    foreach(ADVSequenceWidgetAction* action, wActions) {
        if (action->objectName() == objectName()) {
            return action;
        }
    }
    return NULL;
}

QList<DNASequenceObject*> ADVSequenceWidget::getSequenceObjects() const {
    QList<DNASequenceObject*> res;
    foreach(ADVSequenceObjectContext* ctx, seqContexts) {
        res.append(ctx->getSequenceObject());
    }
    return res;
}

void ADVSequenceWidget::buildPopupMenu(QMenu& m) {
    Q_UNUSED(m);
}

}//namespace

