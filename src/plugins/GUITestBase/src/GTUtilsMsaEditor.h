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

#ifndef _U2_GT_UTILS_MSA_EDITOR_H_
#define _U2_GT_UTILS_MSA_EDITOR_H_

#include <QColor>
#include <QRect>

namespace U2 {

class MSAEditor;
class MSAEditorConsensusArea;
class MSAEditorNameList;
class MSAEditorSequenceArea;
class MSAEditorTreeViewerUI;
class MSAEditorUI;
class MSAGraphOverview;
class MSASimpleOverview;
class U2OpStatus;

// If you can't find an appropriate method check the GTUtilsMsaEditorSequenceArea class
class GTUtilsMsaEditor {
public:
    static QColor getGraphOverviewPixelColor(U2OpStatus &os, const QPoint &point);
    static QColor getSimpleOverviewPixelColor(U2OpStatus &os, const QPoint &point);

    static MSAEditor * getEditor(U2OpStatus &os);
    static MSAEditorUI * getEditorUi(U2OpStatus &os);
    static MSAGraphOverview * getGraphOverview(U2OpStatus &os);
    static MSASimpleOverview * getSimpleOverview(U2OpStatus &os);
    static MSAEditorTreeViewerUI * getTreeView(U2OpStatus &os);
    static MSAEditorNameList * getNameListArea(U2OpStatus &os);
    static MSAEditorConsensusArea * getConsensusArea(U2OpStatus &os);
    static MSAEditorSequenceArea * getSequenceArea(U2OpStatus &os);

    static QRect getSequenceNameRect(U2OpStatus &os, const QString &sequenceName);
    static QRect getColumnHeaderRect(U2OpStatus &os, int column);

    static void replaceSequence(U2OpStatus &os, const QString &sequenceToReplace, int targetPosition);
    static void removeColumn(U2OpStatus &os, int column);

    static void clickSequenceName(U2OpStatus &os, const QString &sequenceName, Qt::MouseButton mouseButton = Qt::LeftButton);
    static void clickColumn(U2OpStatus &os, int column, Qt::MouseButton mouseButton = Qt::LeftButton);

    static QString getReferenceSequenceName(U2OpStatus &os);
    static void setReference(U2OpStatus &os, const QString &sequenceName);

    static void toggleCollapsingMode(U2OpStatus &os);
    static void toggleCollapsingGroup(U2OpStatus &os, const QString &groupName);
    static bool isSequenceCollapsed(U2OpStatus &os, const QString &seqName);

    static int getSequencesCount(U2OpStatus &os);

    static void undo(U2OpStatus &os);
    static void redo(U2OpStatus &os);

    static void buildPhylogeneticTree(U2OpStatus &os, const QString &pathToSave);
};

}   // namespace U2

#endif // _U2_GT_UTILS_MSA_EDITOR_H_
