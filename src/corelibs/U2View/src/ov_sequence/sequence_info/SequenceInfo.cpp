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

#include "SequenceInfo.h"

#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/ShowHideSubgroupWidget.h>

#include <U2View/ADVSequenceWidget.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2Gui/HelpButton.h>


namespace U2 {


const int SequenceInfo::COMMON_STATISTICS_VALUE_MAX_WIDTH = 90;
const QString SequenceInfo::CAPTION_SEQ_REGION_LENGTH = "Length: ";

//nucl
const QString SequenceInfo::CAPTION_SEQ_GC_CONTENT = "GC Content: ";
const QString SequenceInfo::CAPTION_SEQ_MOLAR_WEIGHT = "Molar Weight: ";
const QString SequenceInfo::CAPTION_SEQ_MOLAR_EXT_COEF = "Molat Ext. Coef: ";
const QString SequenceInfo::CAPTION_SEQ_MELTING_TM = "Melting TM: ";

const QString SequenceInfo::CAPTION_SEQ_NMOLE_OD = "nmole/OD<sub>260</sub> : ";
const QString SequenceInfo::CAPTION_SEQ_MG_OD = QChar(0x3BC) + QString("g/OD<sub>260</sub> : "); // 0x3BC - greek 'mu'

//amino
const QString SequenceInfo::CAPTION_SEQ_MOLECULAR_WEIGHT = "Molecular Weight: ";
const QString SequenceInfo::CAPTION_SEQ_ISOELECTIC_POINT = "Isoelectic Point: ";

const QString SequenceInfo::CHAR_OCCUR_GROUP_ID = "char_occur_group";
const QString SequenceInfo::DINUCL_OCCUR_GROUP_ID = "dinucl_occur_group";
const QString SequenceInfo::STAT_GROUP_ID = "stat_group";


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

    // Common statistics
    statisticLabel = new QLabel(this);
    statsWidget = new ShowHideSubgroupWidget(STAT_GROUP_ID, tr("Common Statistics"), statisticLabel, true);
    statsWidget->setObjectName("Common Statistics");
    SAFE_POINT(statsWidget->layout() != NULL, tr("No layout in ShowHideSubgroupWidget"),);
    statsWidget->layout()->setContentsMargins(0, 0, 0, 0);

    mainLayout->addWidget(statsWidget);

    // Characters occurrence
    charOccurLabel = new QLabel(this);
    charOccurWidget = new ShowHideSubgroupWidget(
        CHAR_OCCUR_GROUP_ID, tr("Characters Occurrence"), charOccurLabel, true);
    charOccurWidget->setObjectName("Characters Occurrence");

    mainLayout->addWidget(charOccurWidget);

    // Dinucleotides
    dinuclLabel = new QLabel(this);
    dinuclWidget = new ShowHideSubgroupWidget(
        DINUCL_OCCUR_GROUP_ID, tr("Dinucleotides"), dinuclLabel, false);
    dinuclWidget->setObjectName("Dinucleotides");

    mainLayout->addWidget(dinuclWidget);

    QHBoxLayout* helpLayout = new QHBoxLayout();
    QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal, this);
    helpLayout->addWidget(buttonBox);
    new HelpButton(this, buttonBox, "4227298");

    mainLayout->addLayout(helpLayout);

    // Make some labels selectable by a user (so he could copy them)
    charOccurLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    dinuclLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    statisticLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

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
        const DNAAlphabet* activeSequenceAlphabet = activeSequenceContext->getAlphabet();
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

    const DNAAlphabet* activeSequenceAlphabet = activeSequenceContext->getAlphabet();
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
    connect(&dnaStatisticsTaskRunner, SIGNAL(si_finished()), SLOT(sl_updateStatData()));

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
    // Launch the statistics, characters and dinucleotides calculation tasks,
    // if corresponding groups are present and opened
    ADVSequenceObjectContext* activeContext = annotatedDnaView->getSequenceInFocus();
    SAFE_POINT(0 != activeContext, "A sequence context is NULL!",);

    U2SequenceObject* seqObj = activeContext->getSequenceObject();
    U2EntityRef seqRef = seqObj->getSequenceRef();
    const DNAAlphabet* alphabet = activeContext->getAlphabet();

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

    if (subgroupId.isEmpty() || subgroupId == STAT_GROUP_ID)
    {
        if ((!statsWidget->isHidden()) && (statsWidget->isSubgroupOpened()))
        {
            statsWidget->showProgress();

            dnaStatisticsTaskRunner.run(new DNAStatisticsTask(activeContext, currentRegion));
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

void SequenceInfo::sl_updateStatData() {
    statsWidget->hideProgress();

    DNAStatistics stats = dnaStatisticsTaskRunner.getResult();

    ADVSequenceWidget *wgt = annotatedDnaView->getSequenceWidgetInFocus();
    SAFE_POINT(wgt != NULL, tr("Sequence widget is NULL"), );
    ADVSequenceObjectContext *ctx = wgt->getActiveSequenceContext();
    SAFE_POINT(ctx != NULL, tr("Sequence context is NULL"), );
    SAFE_POINT(ctx->getAlphabet() != NULL, tr("Sequence alphbet is NULL"), );

    QString statsInfo = "<table cellspacing=5>";
    statsInfo += formTableRow( CAPTION_SEQ_REGION_LENGTH,  getFormattedLongNumber(stats.length) );
    if (ctx->getAlphabet()->isNucleic()) {
        statsInfo += formTableRow( CAPTION_SEQ_GC_CONTENT, QString::number(stats.gcContent, 'f', 2) + "%");
        statsInfo += formTableRow( CAPTION_SEQ_MOLAR_WEIGHT, QString::number(stats.molarWeight, 'f', 2) + " Da");
        statsInfo += formTableRow( CAPTION_SEQ_MOLAR_EXT_COEF, QString::number(stats.molarExtCoef) + " I/mol");
        statsInfo += formTableRow( CAPTION_SEQ_MELTING_TM, QString::number(stats.meltingTm, 'f', 2) + " C");

        statsInfo += formTableRow( CAPTION_SEQ_NMOLE_OD, QString::number(stats.nmoleOD260, 'f', 2));
        statsInfo += formTableRow( CAPTION_SEQ_MG_OD, QString::number(stats.mgOD260, 'f', 2));
    } else if (ctx->getAlphabet()->isAmino()) {
        statsInfo += formTableRow( CAPTION_SEQ_MOLECULAR_WEIGHT, QString::number(stats.molecularWeight, 'f', 2));
        statsInfo += formTableRow( CAPTION_SEQ_ISOELECTIC_POINT, QString::number(stats.isoelectricPoint, 'f', 2));
    }

    statsInfo += "</table>";

    statisticLabel->setText(statsInfo);
}

QString SequenceInfo::formTableRow(const QString& caption, QString value) const {
    QString result;

    QFontMetrics metrics = statisticLabel->fontMetrics();
    result = "<tr><td><b>" + tr("%1").arg(caption) + "</b></td><td>"
            + metrics.elidedText(value, Qt::ElideRight, COMMON_STATISTICS_VALUE_MAX_WIDTH)
            + "</td></tr>";
    return result;
}


} // namespace
