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

#ifndef U2_GTUTILS_OPTION_PANEL_MSA_H
#define U2_GTUTILS_OPTION_PANEL_MSA_H

#include "GTGlobals.h"

class QLineEdit;
class QPushButton;
class QToolButton;

namespace U2 {

class GTUtilsOptionPanelMsa{
public:
    enum Tabs{General, Highlighting, PairwiseAlignment, TreeSettings, ExportConsensus, Statistics};
    enum AddRefMethod{Button, Completer};

    static const QMap<Tabs, QString> tabsNames;
    static const QMap<Tabs, QString> innerWidgetNames;

    static void toggleTab(HI::GUITestOpStatus &os, Tabs tab);
    static void openTab(HI::GUITestOpStatus &os, Tabs tab);
    static void closeTab(HI::GUITestOpStatus &os, Tabs tab);
    static bool isTabOpened(HI::GUITestOpStatus &os, Tabs tab);

    static void addReference(HI::GUITestOpStatus &os, QString seqName, AddRefMethod method = Button);
    static void addFirstSeqToPA(HI::GUITestOpStatus &os, QString seqName, AddRefMethod method = Button);
    static void addSecondSeqToPA(HI::GUITestOpStatus &os, QString seqName, AddRefMethod method = Button);
    static QString getSeqFromPAlineEdit(HI::GUITestOpStatus &os, int num);
    static void removeReference(HI::GUITestOpStatus &os);
    static QString getReference(HI::GUITestOpStatus &os);
    static int getLength(HI::GUITestOpStatus &os);
    static int getHeight(HI::GUITestOpStatus &os);

    static void setColorScheme(HI::GUITestOpStatus &os, const QString &colorSchemeName);
    static QString getColorScheme(HI::GUITestOpStatus &os);

    //functions for accessing PA gui elements
    static QToolButton* getAddButton(HI::GUITestOpStatus &os, int number);
    static QLineEdit* getSeqLineEdit(HI::GUITestOpStatus &os, int number);
    static QToolButton* getDeleteButton(HI::GUITestOpStatus &os, int number);
    static QPushButton *getAlignButton(HI::GUITestOpStatus &os);
    static void setPairwiseAlignmentAlgorithm(HI::GUITestOpStatus &os, const QString &algorithm);

private:
    static QWidget* getWidget(HI::GUITestOpStatus &os, const QString& widgetName, int number);

    static void addSeqToPA(HI::GUITestOpStatus &os, QString seqName, AddRefMethod method, int number);


private:
    static QMap<Tabs, QString> initNames();
    static QMap<Tabs, QString> initInnerWidgetNames();
};


}



#endif // GTUTILSOPTIONPANELMSA_H
