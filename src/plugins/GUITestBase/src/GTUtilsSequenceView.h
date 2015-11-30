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

#ifndef _U2_GT_UTILS_SEQUENCE_VIEW_H
#define _U2_GT_UTILS_SEQUENCE_VIEW_H

#include <QString>
#include <GTGlobals.h>

namespace U2 {

class ADVSingleSequenceWidget;
class PanView;
class U2Region;
class Overview;
class GSequenceGraphView;
class TextLabel;

class GTUtilsSequenceView {
public:
    static void getSequenceAsString(HI::GUITestOpStatus &os, QString &sequence);
    static QString getSequenceAsString(HI::GUITestOpStatus &os, int number = 0);
    static QString getBeginOfSequenceAsString(HI::GUITestOpStatus &os, int length);
    static QString getEndOfSequenceAsString(HI::GUITestOpStatus &os, int length);
    static int getLengthOfSequence(HI::GUITestOpStatus &os);
    static int getVisiableStart(HI::GUITestOpStatus &os, int widgetNumber = 0);
    static U2Region getVisibleRange(HI::GUITestOpStatus &os, int widgetNumber = 0);
    static void checkSequence(HI::GUITestOpStatus &os, const QString &expectedSequence);
    static void selectSequenceRegion(HI::GUITestOpStatus &os, int from, int to);

    static void openSequenceView(HI::GUITestOpStatus &os, const QString &sequenceName);
    static void addSequenceView(HI::GUITestOpStatus &os, const QString &sequenceName);

    static void goToPosition(HI::GUITestOpStatus &os, int position);

    static ADVSingleSequenceWidget * getSeqWidgetByNumber(HI::GUITestOpStatus &os, int number = 0, const GTGlobals::FindOptions& = GTGlobals::FindOptions());
    static PanView * getPanViewByNumber(HI::GUITestOpStatus &os, int number = 0, const GTGlobals::FindOptions& = GTGlobals::FindOptions());
    static Overview* getOverviewByNumber(HI::GUITestOpStatus &os, int number = 0, const GTGlobals::FindOptions& = GTGlobals::FindOptions());
    static int getSeqWidgetsNumber(HI::GUITestOpStatus &os);
    static QVector<U2Region> getSelection(HI::GUITestOpStatus &os, int number = 0);
    static QString getSeqName(HI::GUITestOpStatus &os, int number = 0);
    static QString getSeqName(HI::GUITestOpStatus &os, ADVSingleSequenceWidget *seqWidget);

    //clicks on simple annotation on sequence view
    static void clickAnnotationDet(HI::GUITestOpStatus &os, QString name, int startpos, int number = 0, Qt::MouseButton button = Qt::LeftButton);
    static void clickAnnotationPan(HI::GUITestOpStatus &os, QString name, int startpos, int number = 0, Qt::MouseButton button = Qt::LeftButton);

    static GSequenceGraphView* getGraphView(HI::GUITestOpStatus &os);
    static QList<QVariant> getLabelPositions(HI::GUITestOpStatus &os, GSequenceGraphView* graph);
    static QList<TextLabel*> getGraphLabels(HI::GUITestOpStatus &os, GSequenceGraphView* graph);
    static QColor getGraphColor(HI::GUITestOpStatus &os, GSequenceGraphView* graph);
};

} // namespace U2

#endif // GTSEQUENCEVIEWUTILS_H
