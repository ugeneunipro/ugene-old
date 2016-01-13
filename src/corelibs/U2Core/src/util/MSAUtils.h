/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

    static bool equalsIgnoreGaps(const MAlignmentRow& row, int startPos, const QByteArray& pattern, int &alternateLen);

    static int getPatternSimilarityIgnoreGaps(const MAlignmentRow& row, int startPos, const QByteArray& pattern, int &alternateLen);

    static MAlignment seq2ma(const QList<GObject*>& dnas, U2OpStatus& os, bool useGenbankHeader = false);

    static MAlignment seq2ma(const QList<DNASequence>& dnas, U2OpStatus& os);

    static QList<DNASequence> ma2seq(const MAlignment& ma, bool trimGaps);

    // sets alphabet if no alignment alphabet was set; checks is the new alphabet equal old alphabet, otherwise sets error
    static void updateAlignmentAlphabet(MAlignment& ma, const DNAAlphabet* a, U2OpStatus& os);

    // Returns row index or -1 if name is not present
    static int getRowIndexByName(const MAlignment& ma, const QString& name);

    //checks that alignment is not empty and all packed sequence parts has equal length
    static bool checkPackedModelSymmetry(MAlignment& ali, U2OpStatus& ti);

    static MAlignmentObject * seqDocs2msaObj(QList<Document*> doc, const QVariantMap& hints, U2OpStatus& os);
    static MAlignmentObject * seqObjs2msaObj(const QList<GObject*>& objects, const QVariantMap& hints, U2OpStatus& os, bool shallowCopy = false);

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
    static void copyRowFromSequence(MAlignmentObject *msaObj, U2SequenceObject *seqObj, U2OpStatus &os);
    static U2MsaRow copyRowFromSequence(U2SequenceObject *seqObj, const U2DbiRef &dstDbi, U2OpStatus &os);
    static U2MsaRow copyRowFromSequence(DNASequence seq, const U2DbiRef &dstDbi, U2OpStatus &os);

    static MAlignment setUniqueRowNames(const MAlignment& ma);
    /**
      * Renames rows in the 'ma' to 'names' according to the following assumptions:
      *   1) 'ma' row names are integers from [0..n - 1] interval,
      *      where n is equal to row number of 'ma';
      *   2) Row name 'i' will be renamed to 'names[i]' value;
      **/
    static bool restoreRowNames(MAlignment& ma, const QStringList& names);
};


}//namespace

#endif
