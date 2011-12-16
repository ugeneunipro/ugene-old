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

#include <U2Core/AppContext.h>
#include <U2Core/U2Region.h>
#include <U2Core/U2SafePoints.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>


namespace U2 {


const QString SequenceInfo::CAPTION_SEQ_REGION_LENGTH = "Length:";


SequenceInfo::SequenceInfo(AnnotatedDNAView* _annotatedDnaView)
    : annotatedDnaView(_annotatedDnaView)
{
    QList<ADVSequenceObjectContext*> seqContexts = annotatedDnaView->getSequenceContexts();
    SAFE_POINT(!seqContexts.empty(), "AnnotatedDNAView has no sequences contexts!",);

    // Listen when something has been changed in the AnnotatedDNAView
    connect(annotatedDnaView, SIGNAL(si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*)),
        this, SLOT(sl_onFocusChanged(ADVSequenceWidget*, ADVSequenceWidget*)));

    connect(annotatedDnaView, SIGNAL(si_sequenceModified(ADVSequenceObjectContext*)),
        this, SLOT(sl_onSequenceModified(ADVSequenceObjectContext*)));

    // Set the layout of the sequence info widget
    QFormLayout* mainLayout = new QFormLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(5);

    QLabel* captionSeqRegionLength = new QLabel(CAPTION_SEQ_REGION_LENGTH);

    sequenceRegionLength = new QLabel(getStrLengthOfSeqInFocus());

    mainLayout->addRow(captionSeqRegionLength, sequenceRegionLength);
    setLayout(mainLayout);
}


void SequenceInfo::sl_onSelectionChanged(LRegionsSelection*, const QVector<U2Region>& , const QVector<U2Region>&)
{
    sequenceRegionLength->setText("hello");
}


void SequenceInfo::sl_onSequenceModified(ADVSequenceObjectContext* /* seqContext */)
{
    sequenceRegionLength->setText(getStrLengthOfSeqInFocus());
}


void SequenceInfo::sl_onFocusChanged(ADVSequenceWidget* /* prevWidget */, ADVSequenceWidget* /* currentWidget */)
{
    sequenceRegionLength->setText(getStrLengthOfSeqInFocus());
}


QString SequenceInfo::getStrLengthOfSeqInFocus()
{
    ADVSequenceObjectContext* activeSequenceContext = annotatedDnaView->getSequenceInFocus();

    if (activeSequenceContext != 0)
    {
        return QString::number(activeSequenceContext->getSequenceLength());
    }
    else
    {
        return QString("");
    }
}


} // namespace
