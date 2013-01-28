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

#ifndef _U2_MSA_UTILS_H_
#define _U2_MSA_UTILS_H_

#include <U2Core/global.h>
#include <U2Core/DNASequence.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/MAlignmentObject.h>

namespace U2 {

class GObject;
class MAlignment;
class MAlignmentRow;
class U2OpStatus;

class U2CORE_EXPORT MSAUtils : public QObject {
    Q_OBJECT
public:

    static bool equalsIgnoreGaps(const MAlignmentRow& row, int startPos, const QByteArray& pattern);
    
    static MAlignment seq2ma(const QList<GObject*>& dnas, U2OpStatus& os, bool useGenbankHeader = false);

    static MAlignment seq2ma(const QList<DNASequence>& dnas, U2OpStatus& os);

    static QList<DNASequence> ma2seq(const MAlignment& ma, bool trimGaps);

    // Returns row index or -1 if name is not present
    static int getRowIndexByName(const MAlignment& ma, const QString& name);

    //checks that alignment is not empty and all packed sequence parts has equal length
    static bool checkPackedModelSymmetry(MAlignment& ali, U2OpStatus& ti);

    static MAlignmentObject* seqDocs2msaObj(QList<Document*> doc, U2OpStatus& os, bool useGenbankHeader = false);
    static MAlignmentObject* seqObjs2msaObj(const QList<GObject*>& objects, U2OpStatus& os, bool useGenbankHeader = false);

    /**
     * Compares rows in the 'origMsa' and 'newMsa' by names of the sequences.
     * It is assumed that:
     *   1) All rows in 'newMsa' are present in 'origMsa';
     *   2) Corresponding sequences are the same (however, only their length are verified);
     * The method modifies 'newMsa' to contain original rows and sequences IDs (from 'origMsa') and
     * returns the list of rows IDs in the order of 'newMsa'.
     * Note, that 'newMsa' may contain less rows than 'origMsa'
     * (e.g. ClustalO may decrease the number of sequences after the alignment).
     */
    static QList<qint64> compareRowsAfterAlignment(const MAlignment& origMsa, MAlignment& newMsa, U2OpStatus& os);
};


}//namespace

#endif
