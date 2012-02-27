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

#include "SequenceInfo.h"

#include <U2Gui/ShowHideSubgroupWidget.h>

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2SafePoints.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>

#include <QPointer>


namespace U2 {


const QString SequenceInfo::CAPTION_SEQ_REGION_LENGTH = "Length: ";

const QString SequenceInfo::CHAR_OCCUR_GROUP_ID = "char_occur_group";
const QString SequenceInfo::DINUCL_OCCUR_GROUP_ID = "dinucl_occur_group";


SequenceInfo::SequenceInfo(AnnotatedDNAView* _annotatedDnaView)
    : annotatedDnaView(_annotatedDnaView)
{
    SAFE_POINT(0 != annotatedDnaView, "AnnotatedDNAView is NULL!",);

    updateCurrentRegion();
    initLayout();
    connectSlots();
    launchCalculations();
}


void SequenceInfo::initLayout()
{
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->setAlignment(Qt::AlignTop);

    setLayout(mainLayout);

    // Length
    QFormLayout* lengthLayout = new QFormLayout();
    QLabel* captionSeqRegionLength = new QLabel(CAPTION_SEQ_REGION_LENGTH);
    qint64 length = currentRegion.length;
    sequenceRegionLength = new QLabel(QString::number(length));
    lengthLayout->addRow(captionSeqRegionLength, sequenceRegionLength);
    mainLayout->addLayout(lengthLayout);

    // Characters occurrence
    charOccurLabel = new QLabel(this);
    charOccurWidget = new ShowHideSubgroupWidget(
        CHAR_OCCUR_GROUP_ID, tr("Characters Occurrence"), charOccurLabel, true);

    mainLayout->addWidget(charOccurWidget);

    // Dinucleotides
    dinuclLabel = new QLabel(this);
    dinuclWidget = new ShowHideSubgroupWidget(
        DINUCL_OCCUR_GROUP_ID, tr("Dinucleotides"), dinuclLabel, false);

    mainLayout->addWidget(dinuclWidget);

    updateLayout();
}


void SequenceInfo::updateLayout()
{
    updateCharOccurLayout();
    updateDinuclLayout();
}


void SequenceInfo::updateCharOccurLayout()
{
    ADVSequenceObjectContext* activeSequenceContext = annotatedDnaView->getSequenceInFocus();
    if (0 != activeSequenceContext)
    {
        DNAAlphabet* activeSequenceAlphabet = activeSequenceContext->getAlphabet();
        SAFE_POINT(0 != activeSequenceAlphabet, "An active sequence alphabet is NULL!",);

        if ((activeSequenceAlphabet->isNucleic()) ||
            (activeSequenceAlphabet->isAmino()))
        {
            charOccurWidget->show();
        }
        else
        {
            // Do not show the characters occurrence for raw alphabet
            charOccurWidget->hide();
        }
    }
}


void SequenceInfo::updateDinuclLayout()
{
    ADVSequenceObjectContext* activeSequenceContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(0 != activeSequenceContext, "A sequence context is NULL!",);

    DNAAlphabet* activeSequenceAlphabet = activeSequenceContext->getAlphabet();
    SAFE_POINT(0 != activeSequenceAlphabet, "An active sequence alphabet is NULL!",);

    QString alphabetId = activeSequenceAlphabet->getId();

    if ((alphabetId == BaseDNAAlphabetIds::NUCL_DNA_DEFAULT()) ||
        (alphabetId == BaseDNAAlphabetIds::NUCL_RNA_DEFAULT()))
    {
        dinuclWidget->show();
    }
    else
    {
        dinuclWidget->hide();
    }
}


void SequenceInfo::connectSlotsForSeqContext(ADVSequenceObjectContext* seqContext)
{
    SAFE_POINT(seqContext, "A sequence context is NULL!",);

    connect(seqContext->getSequenceSelection(),
        SIGNAL(si_selectionChanged(LRegionsSelection*, const QVector<U2Region>&, const QVector<U2Region>&)),
        SLOT(sl_onSelectionChanged(LRegionsSelection*, const QVector<U2Region>&, const QVector<U2Region>&)));
}


void SequenceInfo::connectSlots()
{
    QList<ADVSequenceObjectContext*> seqContexts = annotatedDnaView->getSequenceContexts();
    SAFE_POINT(!seqContexts.empty(), "AnnotatedDNAView has no sequences contexts!",);

    // A sequence has been selected in the Sequence View
    connect(annotatedDnaView, SIGNAL(si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*)),
        this, SLOT(sl_onFocusChanged(ADVSequenceWidget*, ADVSequenceWidget*)));

    // A sequence has been modified (a subsequence added, removed, etc.)
    connect(annotatedDnaView, SIGNAL(si_sequenceModified(ADVSequenceObjectContext*)),
        this, SLOT(sl_onSequenceModified(ADVSequenceObjectContext*)));

    // A user has selected a sequence region
    foreach (ADVSequenceObjectContext* seqContext, seqContexts) {
        connectSlotsForSeqContext(seqContext);
    }

    // A sequence object has been added
    connect(annotatedDnaView, SIGNAL(si_sequenceAdded(ADVSequenceObjectContext*)),
        SLOT(sl_onSequenceAdded(ADVSequenceObjectContext*)));

    // Calculations have been finished
    connect(&charOccurTaskRunner, SIGNAL(si_finished()), SLOT(sl_updateCharOccurData()));
    connect(&dinuclTaskRunner, SIGNAL(si_finished()), SLOT(sl_updateDinuclData()));

    // A subgroup has been opened/closed
    connect(charOccurWidget, SIGNAL(si_subgroupStateChanged(QString)), SLOT(sl_subgroupStateChanged(QString)));
    connect(dinuclWidget, SIGNAL(si_subgroupStateChanged(QString)), SLOT(sl_subgroupStateChanged(QString)));
}


void SequenceInfo::sl_onSelectionChanged(LRegionsSelection*,
                                         const QVector<U2Region>& added,
                                         const QVector<U2Region>& removed)
{
    // Each time selection is changed the signal is emitted
    // with the whole previously selected region(s) removed and
    // the whole currently selected region(s) added,
    // e.g. "removed: 3..10, added: 3..11"
    if (!added.empty())
    {
        // Only the first region is taken into account
        const U2Region& region = added.first();

        // Skip, if a signal with the same region has already came
        if (region == currentRegion)
        {
            return;
        }
        else
        {
            currentRegion = region;
            launchCalculations();
        }
    } else {
        // None is selected
        if (!removed.empty()) {
            updateCurrentRegion();
            launchCalculations();
        }
    }
}


void SequenceInfo::sl_onSequenceModified(ADVSequenceObjectContext* /* seqContext */)
{
    updateCurrentRegion();
    launchCalculations();
}


void SequenceInfo::sl_onFocusChanged(ADVSequenceWidget* /* prevWidget */, ADVSequenceWidget* currentWidget)
{
    if (0 != currentWidget) { // i.e. the sequence has been deleted
        updateCurrentRegion();
        launchCalculations();
    }
}


void SequenceInfo::sl_onSequenceAdded(ADVSequenceObjectContext* seqContext)
{
    connectSlotsForSeqContext(seqContext);
}


void SequenceInfo::sl_subgroupStateChanged(QString subgroupId)
{
    launchCalculations(subgroupId);
}


void SequenceInfo::updateCurrentRegion()
{
    ADVSequenceObjectContext* seqContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(0 != seqContext, "A sequence context is NULL!",);

    DNASequenceSelection* selection = seqContext->getSequenceSelection();

    QVector<U2Region> selectedRegions = selection->getSelectedRegions();
    if (!selectedRegions.empty())
    {
        currentRegion = selectedRegions.first();
    }
    else
    {
        currentRegion = U2Region(0, seqContext->getSequenceLength());
    }
}

/** Formats long number by separating each three digits */
QString getFormattedLongNumber(qint64 num)
{
    QString result;

    int DIVIDER = 1000;
    do {
        int lastThreeDigits = num % DIVIDER;

        QString digitsStr = QString::number(lastThreeDigits);

        // Fill with zeros if the digits are in the middle of the number
        if (num / DIVIDER != 0) {
            digitsStr = QString("%1").arg(digitsStr, 3, '0');
        }

        result = digitsStr + " " + result;

        num /= DIVIDER;
    } while (num != 0);

    return result;
}


void SequenceInfo::launchCalculations(QString subgroupId)
{
    // The length is shown for all sequences
    if (subgroupId.isEmpty())
    {
        sequenceRegionLength->setText(getFormattedLongNumber(currentRegion.length));
    }

    // Launch the characters and dinucleotides calculation tasks,
    // if corresponding groups are present and opened
    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(0 != activeContext, "A sequence context is NULL!",);

    U2SequenceObject* seqObj = activeContext->getSequenceObject();
    U2EntityRef seqRef = seqObj->getSequenceRef();
    DNAAlphabet* alphabet = activeContext->getAlphabet();

    if (subgroupId.isEmpty() || subgroupId == CHAR_OCCUR_GROUP_ID)
    {
        if ((!charOccurWidget->isHidden()) && (charOccurWidget->isSubgroupOpened()))
        {
            charOccurWidget->showProgress();

            charOccurTaskRunner.run(new CharOccurTask(alphabet, seqRef, currentRegion));
        }
    }
    
    if (subgroupId.isEmpty() || subgroupId == DINUCL_OCCUR_GROUP_ID)
    {
        if ((!dinuclWidget->isHidden()) && (dinuclWidget->isSubgroupOpened()))
        {
            dinuclWidget->showProgress();

            dinuclTaskRunner.run(new DinuclOccurTask(alphabet, seqRef, currentRegion));
        }
    }
}


void SequenceInfo::sl_updateCharOccurData()
{
    charOccurWidget->hideProgress();

    QList<CharOccurResult> charOccurResults = charOccurTaskRunner.getResult();

    QString charOccurInfo = "<table cellspacing=5>";
    foreach (CharOccurResult result, charOccurResults) {
        charOccurInfo += "<tr>";
        charOccurInfo += QString("<td><b>") + result.getChar() + QString(":&nbsp;&nbsp;</td>");
        charOccurInfo += "<td>" + getFormattedLongNumber(result.getNumberOfOccur()) + "&nbsp;&nbsp;</td>";
        charOccurInfo += "<td>" + QString::number(result.getPercentage(), 'f', 1) + "%" + "&nbsp;&nbsp;</td>";
        charOccurInfo += "</tr>";
    }
    charOccurInfo += "</table>";

    charOccurLabel->setText(charOccurInfo);
}


void SequenceInfo::sl_updateDinuclData()
{
    dinuclWidget->hideProgress();

    QMap<QByteArray, qint64> dinuclOccurrence = dinuclTaskRunner.getResult();

    QMap<QByteArray, qint64>::const_iterator i = dinuclOccurrence.constBegin();
    QString dinuclInfo = "<table cellspacing=5>";
    while (i != dinuclOccurrence.constEnd()) {
        dinuclInfo += "<tr>";
        dinuclInfo += QString("<td><b>") + i.key() + QString(":&nbsp;&nbsp;</td>");
        dinuclInfo += "<td>" + getFormattedLongNumber(i.value()) + "&nbsp;&nbsp;</td>";
        dinuclInfo += "</tr>";
        ++i;
    }
    dinuclInfo += "</table>";

    dinuclLabel->setText(dinuclInfo);
}


} // namespace
