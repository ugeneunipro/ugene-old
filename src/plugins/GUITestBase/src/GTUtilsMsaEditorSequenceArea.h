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
    static MSAEditorSequenceArea * getSequenceArea(HI::GUITestOpStatus &os);
    static void callContextMenu(HI::GUITestOpStatus &os, const QPoint &innerCoords = QPoint());  // zero-based position

    static void checkSelectedRect(HI::GUITestOpStatus &os, const QRect &expectedRect);
    static void checkSorted(HI::GUITestOpStatus &os, bool sortedState = true);

    static void checkConsensus(HI::GUITestOpStatus &os, QString cons);
    // may be used for selecting visible columns only
    static void selectColumnInConsensus( HI::GUITestOpStatus &os, int columnNumber );

    // MSAEditorNameList
    static QStringList getNameList(HI::GUITestOpStatus &os);
    static QStringList getVisibaleNames(HI::GUITestOpStatus &os);
    static QString getSimilarityValue(HI::GUITestOpStatus &os, int row);
    static void clickCollapceTriangle(HI::GUITestOpStatus &os, QString seqName);
    static bool isCollapsed(HI::GUITestOpStatus &os, QString seqName);
    static bool collapsingMode(HI::GUITestOpStatus &os);

    static int getLeftOffset(HI::GUITestOpStatus &os);
    static int getRightOffset(HI::GUITestOpStatus &os);

    static int getLength(HI::GUITestOpStatus &os);
    static int getNumVisibleBases(HI::GUITestOpStatus &os);

    static QRect getSelectedRect(HI::GUITestOpStatus &os);

    static void moveTo(HI::GUITestOpStatus &os, const QPoint &p);

    // selects area in MSA coordinats, if some p coordinate less than 0, it becomes max valid coordinate
    // zero-based position
    static void selectArea(HI::GUITestOpStatus &os, QPoint p1 = QPoint(0, 0), QPoint p2 = QPoint(-1, -1));
    static void cancelSelection(HI::GUITestOpStatus &os);
    static QPoint convertCoordinates(HI::GUITestOpStatus &os, const QPoint p);
    static void click(HI::GUITestOpStatus &os, QPoint p = QPoint(0, 0));

    // scrolls to the position (in the MSA zero-based coordinates)
    static void scrollToPosition(HI::GUITestOpStatus &os, const QPoint& position);
    static void scrollToBottom(HI::GUITestOpStatus &os);
    static void clickToPosition(HI::GUITestOpStatus &os, const QPoint& position);

    static void selectSequence(HI::GUITestOpStatus &os, const QString &seqName);
    static bool isSequenceSelected(HI::GUITestOpStatus &os, const QString &seqName);
    static void removeSequence(HI::GUITestOpStatus &os, const QString &sequenceName);
    static int getSelectedSequencesNum(HI::GUITestOpStatus &os);
    static bool isSequenceVisible(HI::GUITestOpStatus &os, const QString &seqName);
    static QString getSequenceData(HI::GUITestOpStatus &os, const QString &sequenceName);

    static bool offsetsVisible(HI::GUITestOpStatus &os);

    static bool hasAminoAlphabet(HI::GUITestOpStatus &os);
    static bool isSequenceHightighted(HI::GUITestOpStatus &os, const QString& seqName);
    static QString getColor(HI::GUITestOpStatus &os, QPoint p);
    static bool checkColor(HI::GUITestOpStatus &os, const QPoint& p, const QString& expectedColor);
    static int getRowHeight(HI::GUITestOpStatus &os);

    static void renameSequence(HI::GUITestOpStatus &os, const QString& seqToRename, const QString& newName);

    static void createColorScheme(HI::GUITestOpStatus &os, const QString& schemeName, const NewColorSchemeCreator::alphabet al);
    static void deleteColorScheme(HI::GUITestOpStatus &os, const QString& schemeName);

    static void checkSelection(HI::GUITestOpStatus &os, const QPoint& start, const QPoint& end, const QString& expected);

    static const QString highlightningColorName;
};


} // namespace
#endif // GTUTILSMSAEDITORSEQUENCEAREA_H
