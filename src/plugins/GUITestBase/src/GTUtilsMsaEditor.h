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
#include <GTGlobals.h>

namespace U2 {

class MSAEditor;
class MSAEditorConsensusArea;
class MSAEditorNameList;
class MSAEditorSequenceArea;
class MSAEditorTreeViewerUI;
class MSAEditorUI;
class MSAGraphOverview;
class MSASimpleOverview;

// If you can't find an appropriate method check the GTUtilsMsaEditorSequenceArea class
class GTUtilsMsaEditor {
public:
    static QColor getGraphOverviewPixelColor(HI::GUITestOpStatus &os, const QPoint &point);
    static QColor getSimpleOverviewPixelColor(HI::GUITestOpStatus &os, const QPoint &point);

    static MSAEditor * getEditor(HI::GUITestOpStatus &os);
    static MSAEditorUI * getEditorUi(HI::GUITestOpStatus &os);
    static MSAGraphOverview * getGraphOverview(HI::GUITestOpStatus &os);
    static MSASimpleOverview * getSimpleOverview(HI::GUITestOpStatus &os);
    static MSAEditorTreeViewerUI * getTreeView(HI::GUITestOpStatus &os);
    static MSAEditorNameList * getNameListArea(HI::GUITestOpStatus &os);
    static MSAEditorConsensusArea * getConsensusArea(HI::GUITestOpStatus &os);
    static MSAEditorSequenceArea * getSequenceArea(HI::GUITestOpStatus &os);

    static QRect getSequenceNameRect(HI::GUITestOpStatus &os, const QString &sequenceName);
    static QRect getColumnHeaderRect(HI::GUITestOpStatus &os, int column);

    static void replaceSequence(HI::GUITestOpStatus &os, const QString &sequenceToReplace, int targetPosition);
    static void removeColumn(HI::GUITestOpStatus &os, int column);

    static void clickSequenceName(HI::GUITestOpStatus &os, const QString &sequenceName, Qt::MouseButton mouseButton = Qt::LeftButton);
    static void clickColumn(HI::GUITestOpStatus &os, int column, Qt::MouseButton mouseButton = Qt::LeftButton);

    static QString getReferenceSequenceName(HI::GUITestOpStatus &os);
    static void setReference(HI::GUITestOpStatus &os, const QString &sequenceName);

    static void toggleCollapsingMode(HI::GUITestOpStatus &os);
    static void toggleCollapsingGroup(HI::GUITestOpStatus &os, const QString &groupName);
    static bool isSequenceCollapsed(HI::GUITestOpStatus &os, const QString &seqName);

    static int getSequencesCount(HI::GUITestOpStatus &os);

    static void undo(HI::GUITestOpStatus &os);
    static void redo(HI::GUITestOpStatus &os);

    static void buildPhylogeneticTree(HI::GUITestOpStatus &os, const QString &pathToSave);
};

}   // namespace U2

#endif // _U2_GT_UTILS_MSA_EDITOR_H_
