/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "api/GTGlobals.h"

class QLineEdit;
class QPushButton;
class QToolButton;

namespace U2 {

class U2OpStatus;

class GTUtilsOptionPanelMsa{
public:
    enum Tabs{General, Highlighting, PairwiseAlignment, TreeSettings, ExportConsensus, Statistics};
    enum AddRefMethod{Button, Completer};

    static const QMap<Tabs, QString> tabsNames;
    static const QMap<Tabs, QString> innerWidgetNames;
    //Opens option panel tab. TODO: check if tab is already opened
    static void openTab(U2OpStatus &os, Tabs tab);
    static void addReference(U2OpStatus &os, QString seqName, AddRefMethod method = Button);
    static void addFirstSeqToPA(U2OpStatus &os, QString seqName, AddRefMethod method = Button);
    static void addSecondSeqToPA(U2OpStatus &os, QString seqName, AddRefMethod method = Button);
    static void removeReference(U2OpStatus &os);
    static QString getReference(U2OpStatus &os);
    static int getLength(U2OpStatus &os);
    static int getHeight(U2OpStatus &os);

    static void setColorScheme(U2OpStatus &os, const QString &colorSchemeName);
    static QString getColorScheme(U2OpStatus &os);

    //functions for accessing PA gui elements
    static QToolButton* getAddButton(U2OpStatus &os, int number);
    static QLineEdit* getSeqLineEdit(U2OpStatus &os, int number);
    static QToolButton* getDeleteButton(U2OpStatus &os, int number);
    static QPushButton *getAlignButton(U2OpStatus &os);
    static void setPairwiseAlignmentAlgorithm(U2OpStatus &os, const QString &algorithm);

private:
    static QWidget* getWidget(U2OpStatus &os, const QString& widgetName, int number);

    static void addSeqToPA(U2OpStatus &os, QString seqName, AddRefMethod method, int number);


private:
    static QMap<Tabs, QString> initNames();
    static QMap<Tabs, QString> initInnerWidgetNames();
};


}



#endif // GTUTILSOPTIONPANELMSA_H
