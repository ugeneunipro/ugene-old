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
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/L10n.h>
#include <U2Core/MultiTask.h>
#include <U2Core/U2SafePoints.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>

#include "ExtractProductTask.h"
#include "InSilicoPcrTask.h"
#include "PrimerStatistics.h"

#include "InSilicoPcrOptionPanelWidget.h"

namespace U2 {

InSilicoPcrOptionPanelWidget::InSilicoPcrOptionPanelWidget(AnnotatedDNAView *annotatedDnaView)
: QWidget(), annotatedDnaView(annotatedDnaView), pcrTask(NULL)
{
    setupUi(this);
    forwardPrimerBox->setTitle(tr("Forward primer"));
    reversePrimerBox->setTitle(tr("Reverse primer"));

    connect(forwardPrimerBox, SIGNAL(si_primerChanged()), SLOT(sl_onPrimerChanged()));
    connect(reversePrimerBox, SIGNAL(si_primerChanged()), SLOT(sl_onPrimerChanged()));
    connect(findProductButton, SIGNAL(clicked()), SLOT(sl_findProduct()));
    connect(extractProductButton, SIGNAL(clicked()), SLOT(sl_extractProduct()));
    connect(annotatedDnaView, SIGNAL(si_sequenceModified(ADVSequenceObjectContext*)), SLOT(sl_onSequenceChanged(ADVSequenceObjectContext *)));
    connect(annotatedDnaView, SIGNAL(si_sequenceRemoved(ADVSequenceObjectContext*)), SLOT(sl_onSequenceChanged(ADVSequenceObjectContext *)));
    connect(annotatedDnaView, SIGNAL(si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*)), SLOT(sl_onFocusChanged()));
    connect(productsTable->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), SLOT(sl_onProductsSelectionChanged()));
    connect(productsTable, SIGNAL(doubleClicked(const QModelIndex &)), SLOT(sl_onProductDoubleClicked()));

    productsWidget->hide();
    sl_onFocusChanged();
    sl_onPrimerChanged();
}

InSilicoPcrOptionPanelWidget::~InSilicoPcrOptionPanelWidget() {
    if (NULL != pcrTask) {
        pcrTask->cancel();
    }
}

void InSilicoPcrOptionPanelWidget::sl_onPrimerChanged() {
    QByteArray forward = forwardPrimerBox->getPrimer();
    QByteArray reverse = reversePrimerBox->getPrimer();
    bool disabled = forward.isEmpty() || reverse.isEmpty();
    findProductButton->setDisabled(disabled);
    if (disabled) {
        warningLabel->hide();
        return;
    }

    QString message;
    bool correct = PrimerStatistics::checkPcrPrimersPair(forward, reverse, message);
    if (correct) {
        warningLabel->hide();
        findProductButton->setText(tr("Find product(s)"));
    } else {
        warningLabel->show();
        warningLabel->setText(tr("Warning: ") + message);
        findProductButton->setText(tr("Find product(s) anyway"));
    }
}

void InSilicoPcrOptionPanelWidget::sl_findProduct() {
    int maxProduct = productSizeSpinBox->value();
    SAFE_POINT(maxProduct > 0, "Non-positive product size", );
    ADVSequenceObjectContext *sequenceContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(NULL != sequenceContext, L10N::nullPointerError("Sequence Context"), );
    U2SequenceObject *sequenceObject = sequenceContext->getSequenceObject();
    SAFE_POINT(NULL != sequenceObject, L10N::nullPointerError("Sequence Object"), );

    InSilicoPcrTaskSettings settings;
    settings.forwardPrimer = forwardPrimerBox->getPrimer();
    settings.reversePrimer = reversePrimerBox->getPrimer();
    settings.forwardMismatches = forwardPrimerBox->getMismatches();
    settings.reverseMismatches = reversePrimerBox->getMismatches();
    settings.maxProductSize = uint(maxProduct);
    settings.sequence = sequenceObject->getWholeSequenceData();
    settings.sequenceObject = GObjectReference(sequenceObject);
    settings.isCircular = sequenceObject->isCircular();

    pcrTask = new InSilicoPcrTask(settings);
    connect(pcrTask, SIGNAL(si_stateChanged()), SLOT(sl_onFindTaskFinished()));
    AppContext::getTaskScheduler()->registerTopLevelTask(pcrTask);
    setDisabled(true);
    productsWidget->setVisible(false);
}

void InSilicoPcrOptionPanelWidget::sl_onFindTaskFinished() {
    CHECK(sender() == pcrTask, );
    SAFE_POINT(NULL != pcrTask, L10N::nullPointerError("InSilicoPcrTask"), );
    if (pcrTask->isCanceled() || pcrTask->hasError()) {
        disconnect(pcrTask, SIGNAL(si_stateChanged()));
        pcrTask = NULL;
        setEnabled(true);
        return;
    }
    CHECK(pcrTask->isFinished(), );
    showResults(pcrTask);
    pcrTask = NULL;
    setEnabled(true);
}

void InSilicoPcrOptionPanelWidget::showResults(InSilicoPcrTask *task) {
    ADVSequenceObjectContext *sequenceContext = annotatedDnaView->getSequenceContext(task->getSettings().sequenceObject);
    CHECK(NULL != sequenceContext, );

    productsTable->showProducts(task, sequenceContext);
    productsWidget->show();
}

void InSilicoPcrOptionPanelWidget::sl_extractProduct() {
    ADVSequenceObjectContext *sequenceContext = productsTable->productsContext();
    SAFE_POINT(NULL != sequenceContext, L10N::nullPointerError("Sequence Context"), );
    U2SequenceObject *sequenceObject = sequenceContext->getSequenceObject();
    SAFE_POINT(NULL != sequenceObject, L10N::nullPointerError("Sequence Object"), );

    QList<Task*> tasks;
    foreach (const InSilicoPcrProduct &product, productsTable->getSelectedProducts()) {
        tasks << new ExtractProductWrapperTask(product, sequenceContext->getSequenceRef(), sequenceObject->getSequenceName(), sequenceObject->getSequenceLength());
    }
    CHECK(!tasks.isEmpty(), );
    if (1 == tasks.size()) {
        AppContext::getTaskScheduler()->registerTopLevelTask(tasks.first());
    } else {
        AppContext::getTaskScheduler()->registerTopLevelTask(new MultiTask(tr("Extract In Silico PCR products"), tasks));
    }
}

void InSilicoPcrOptionPanelWidget::sl_onSequenceChanged(ADVSequenceObjectContext *sequenceContext) {
    bool tableChanged = productsTable->onSequenceChanged(sequenceContext);
    if (tableChanged) {
        productsWidget->hide();
    }
    CHECK(NULL != pcrTask, );
    bool taskChanged = GObjectReference(sequenceContext->getSequenceGObject()) == pcrTask->getSettings().sequenceObject;
    if (taskChanged) {
        pcrTask->cancel();
    }
}

bool InSilicoPcrOptionPanelWidget::isDnaSequence(ADVSequenceObjectContext *sequenceContext) {
    CHECK(NULL != sequenceContext, false);
    const DNAAlphabet *alphabet = sequenceContext->getAlphabet();
    SAFE_POINT(alphabet != NULL, L10N::nullPointerError("Alphabet"), false);
    return alphabet->isDNA();
}

void InSilicoPcrOptionPanelWidget::sl_onFocusChanged() {
    ADVSequenceObjectContext *sequenceContext = annotatedDnaView->getSequenceInFocus();
    bool isDna = isDnaSequence(sequenceContext);
    runPcrWidget->setEnabled(isDna);
    algoWarningLabel->setVisible(!isDna);
}

void InSilicoPcrOptionPanelWidget::sl_onProductsSelectionChanged() {
    QList<InSilicoPcrProduct> products = productsTable->getSelectedProducts();
    extractProductButton->setEnabled(!products.isEmpty());
}

void InSilicoPcrOptionPanelWidget::sl_onProductDoubleClicked() {
    QList<InSilicoPcrProduct> products = productsTable->getSelectedProducts();
    if (1 == products.size()) {
        sl_extractProduct();
    }
}

} // U2
