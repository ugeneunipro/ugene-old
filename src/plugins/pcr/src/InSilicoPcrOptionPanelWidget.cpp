/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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
#include <U2Core/L10n.h>
#include <U2Core/U2SafePoints.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>

#include "InSilicoPcrTask.h"

#include "InSilicoPcrOptionPanelWidget.h"

namespace U2 {

InSilicoPcrOptionPanelWidget::InSilicoPcrOptionPanelWidget(AnnotatedDNAView *annotatedDnaView)
: QWidget(), annotatedDnaView(annotatedDnaView)
{
    setupUi(this);
    forwardPrimerBox->setTitle(tr("Forward primer"));
    reversePrimerBox->setTitle(tr("Reverse primer"));

    connect(forwardPrimerBox, SIGNAL(si_primerChanged()), SLOT(sl_onPrimerChanged()));
    connect(reversePrimerBox, SIGNAL(si_primerChanged()), SLOT(sl_onPrimerChanged()));
    connect(findProductButton, SIGNAL(clicked()), SLOT(sl_findProduct()));
    connect(extractProductButton, SIGNAL(clicked()), SLOT(sl_extractProduct()));

    productsTable->setVisible(false);
    extractProductButton->setVisible(false);
}

void InSilicoPcrOptionPanelWidget::sl_onPrimerChanged() {

}

void InSilicoPcrOptionPanelWidget::sl_findProduct() {
    int maxProduct = productSizeSpinBox->value();
    SAFE_POINT(maxProduct > 0, "Non-positive product size", );
    ADVSequenceObjectContext *seqContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(NULL != seqContext, L10N::nullPointerError("Sequence Context"), );
    U2SequenceObject *seqObject = seqContext->getSequenceObject();
    SAFE_POINT(NULL != seqObject, L10N::nullPointerError("Sequence Object"), );

    InSilicoPcrTaskSettings settings;
    settings.forwardPrimer = forwardPrimerBox->getPrimer();
    settings.reversePrimer = reversePrimerBox->getPrimer();
    settings.forwardMismatches = forwardPrimerBox->getMismatches();
    settings.reverseMismatches = reversePrimerBox->getMismatches();
    settings.maxProductSize = uint(maxProduct);
    settings.sequence = seqObject->getWholeSequenceData();
    settings.sequenceObject = GObjectReference(seqObject);
    settings.isCircular = seqObject->isCircular();

    InSilicoPcrTask *task = new InSilicoPcrTask(settings);
    connect(task, SIGNAL(si_stateChanged()), SLOT(sl_onFindTaskFinished()));
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
    setDisabled(true);
}

void InSilicoPcrOptionPanelWidget::sl_onFindTaskFinished() {
    InSilicoPcrTask *task = dynamic_cast<InSilicoPcrTask*>(sender());
    SAFE_POINT(NULL != task, L10N::nullPointerError("InSilicoPcrTask"), );
    if (task->isCanceled() || task->hasError()) {
        disconnect(task, SIGNAL(si_stateChanged()));
        setEnabled(true);
        return;
    }
    CHECK(task->isFinished(), );
    setEnabled(true);
    showResults(task);
}

void InSilicoPcrOptionPanelWidget::showResults(InSilicoPcrTask *task) {
    ADVSequenceObjectContext *sequenceContext = annotatedDnaView->getSequenceContext(task->getSettings().sequenceObject);
    CHECK(NULL != sequenceContext, );

    extractProductButton->setVisible(true);
    productsTable->setVisible(true);
    productsTable->showProducts(task, sequenceContext);
}

void InSilicoPcrOptionPanelWidget::sl_extractProduct() {

}

} // U2
