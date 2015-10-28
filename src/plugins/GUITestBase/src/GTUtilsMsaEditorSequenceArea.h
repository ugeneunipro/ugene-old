/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef GTUTILSMSAEDITORSEQUENCEAREA_H
#define GTUTILSMSAEDITORSEQUENCEAREA_H

#include "GTGlobals.h"
#include "runnables/ugene/corelibs/U2Gui/AppSettingsDialogFiller.h"
#include <U2View/MSAEditorSequenceArea.h>

namespace U2 {

class GTUtilsMSAEditorSequenceArea {
public:
    static MSAEditorSequenceArea * getSequenceArea(U2OpStatus &os);
    static void callContextMenu(U2OpStatus &os, const QPoint &innerCoords = QPoint());  // zero-based position

    static void checkSelectedRect(U2OpStatus &os, const QRect &expectedRect);
    static void checkSorted(U2OpStatus &os, bool sortedState = true);

    static void checkConsensus(U2OpStatus &os, QString cons);
    // may be used for selecting visible columns only
    static void selectColumnInConsensus( U2OpStatus &os, int columnNumber );

    // MSAEditorNameList
    static QStringList getNameList(U2OpStatus &os);
    static QStringList getVisibaleNames(U2OpStatus &os);
    static QString getSimilarityValue(U2OpStatus &os, int row);
    static void clickCollapceTriangle(U2OpStatus &os, QString seqName);
    static bool isCollapsed(U2OpStatus &os, QString seqName);
    static bool collapsingMode(U2OpStatus &os);

    static int getLeftOffset(U2OpStatus &os);
    static int getRightOffset(U2OpStatus &os);

    static int getLength(U2OpStatus &os);
    static int getNumVisibleBases(U2OpStatus &os);

    static QRect getSelectedRect(U2OpStatus &os);

    static void moveTo(U2OpStatus &os, const QPoint &p);

    // selects area in MSA coordinats, if some p coordinate less than 0, it becomes max valid coordinate
    // zero-based position
    static void selectArea(U2OpStatus &os, QPoint p1 = QPoint(0, 0), QPoint p2 = QPoint(-1, -1));
    static void cancelSelection(U2OpStatus &os);
    static QPoint convertCoordinates(U2OpStatus &os, const QPoint p);
    static void click(U2OpStatus &os, QPoint p = QPoint(0, 0));

    // scrolls to the position (in the MSA zero-based coordinates)
    static void scrollToPosition(U2OpStatus &os, const QPoint& position);
    static void scrollToBottom(U2OpStatus &os);
    static void clickToPosition(U2OpStatus &os, const QPoint& position);

    static void selectSequence(U2OpStatus &os, const QString &seqName);
    static bool isSequenceSelected(U2OpStatus &os, const QString &seqName);
    static void removeSequence(U2OpStatus &os, const QString &sequenceName);
    static int getSelectedSequencesNum(U2OpStatus &os);
    static bool isSequenceVisible(U2OpStatus &os, const QString &seqName);
    static QString getSequenceData(U2OpStatus &os, const QString &sequenceName);

    static bool offsetsVisible(U2OpStatus &os);

    static bool hasAminoAlphabet(U2OpStatus &os);
    static bool isSequenceHightighted(U2OpStatus &os, const QString& seqName);
    static QString getColor(U2OpStatus &os, QPoint p);
    static bool checkColor(U2OpStatus &os, const QPoint& p, const QString& expectedColor);
    static int getRowHeight(U2OpStatus &os);

    static void renameSequence(U2OpStatus &os, const QString& seqToRename, const QString& newName);

    static void createColorScheme(U2OpStatus &os, const QString& schemeName, const NewColorSchemeCreator::alphabet al);
    static void deleteColorScheme(U2OpStatus &os, const QString& schemeName);

    static void checkSelection(U2OpStatus &os, const QPoint& start, const QPoint& end, const QString& expected);

    static const QString highlightningColorName;
};


} // namespace
#endif // GTUTILSMSAEDITORSEQUENCEAREA_H
