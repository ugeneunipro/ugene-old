/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_SEQUENCE_INFO_H_
#define _U2_SEQUENCE_INFO_H_

#include <QtGui/QtGui>


namespace U2 {

class ADVSequenceObjectContext;
class ADVSequenceWidget;
class AnnotatedDNAView;
class LRegionsSelection;
class U2Region;

class SequenceInfo : public QWidget
{
    Q_OBJECT
public:
    SequenceInfo(AnnotatedDNAView*);

private slots:
    void sl_onSelectionChanged(LRegionsSelection*, const QVector<U2Region>& , const QVector<U2Region>&);

    /** Focus is changed e.g. when a user selects another sequence or deletes the sequence in focus */
    void sl_onFocusChanged(ADVSequenceWidget*, ADVSequenceWidget*);

    /** A sequence part was added, removed or replaced */
    void sl_onSequenceModified(ADVSequenceObjectContext*);

private:
    /**
     * Returns the length of the sequence in focus, converted to QString format.
     * There should be a sequence in focus to call this function!
     */
    QString getStrLengthOfSeqInFocus();

    AnnotatedDNAView* annotatedDnaView;
    QLabel* sequenceRegionLength;

    static const QString CAPTION_SEQ_REGION_LENGTH;
};



} // namespace

#endif
