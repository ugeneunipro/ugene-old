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

#ifndef _U2_GT_UTILS_OPTION_PANEL_SEQUENCE_VIEW_H_
#define _U2_GT_UTILS_OPTION_PANEL_SEQUENCE_VIEW_H_

#include <QToolButton>
#include <QLineEdit>

#include "GTGlobals.h"

namespace U2 {

class U2OpStatus;

class GTUtilsOptionPanelSequenceView{
public:
    enum Tabs {
        Search,
        AnnotationsHighlighting,
        Statistics,
        InSilicoPcr,
        CircularView
    };

    enum AddRefMethod {
        Button,
        Completer
    };

    static const QMap<Tabs,QString> tabsNames;
    static const QMap<Tabs, QString> innerWidgetNames;

    static void toggleTab(U2OpStatus &os, Tabs tab);
    static void openTab(U2OpStatus &os, Tabs tab);
    static void closeTab(U2OpStatus &os, Tabs tab);
    static bool isTabOpened(U2OpStatus &os, Tabs tab);

    // Find pattern options panel tab
    static void enterPattern(U2OpStatus &os, QString pattern, bool useCopyPaste = false);
    static void enterPatternFromFile(U2OpStatus &os, QString filepath, QString filename);

    static bool checkResultsText(U2OpStatus &os, QString expectedText);

    static void setSearchWithAmbiguousBases(U2OpStatus &os, bool searchWithAmbiguousBases = true);
    static void setStrand(U2OpStatus &os, QString strandStr);
    static void setRegionType(U2OpStatus &os, const QString &regionType);
    static void setRegion(U2OpStatus &os, int from, int to);
    static void setSearchInTranslation(U2OpStatus &os, bool inTranslation = true);
    static void setSearchInLocation(U2OpStatus &os, QString strandStr);
    static void setSetMaxResults(U2OpStatus &os, int maxResults);
    static void setAlgorithm(U2OpStatus &os, QString algorithm);
    static void setMatchPercentage(U2OpStatus &os, int percentage);
    static void setUsePatternName(U2OpStatus &os, bool setChecked = true);

    static int getMatchPercentage(U2OpStatus &os);
    static QString getRegionType(U2OpStatus &os);
    static QPair<int, int> getRegion(U2OpStatus &os);
    static const QString getHintText(U2OpStatus &os);

    static void enterFilepathForSavingAnnotations(U2OpStatus &os, QString filepath);

    static void toggleInputFromFilePattern(U2OpStatus &os);

    static void toggleSaveAnnotationsTo(U2OpStatus &os);

    static void clickNext(U2OpStatus &os);
    static void clickPrev(U2OpStatus &os);
    static void clickGetAnnotation(U2OpStatus &os);
    static bool isPrevNextEnabled(U2OpStatus &os);
    static bool isGetAnnotationsEnabled(U2OpStatus &os);

    // Circular view options panel tab
    static void toggleCircularView(U2OpStatus &os);
    static void setTitleFontSize(U2OpStatus &os, int fontSize);
    static int getTitleFontSize(U2OpStatus &os);

    // In Silico PCR tab
    static void setForwardPrimer(U2OpStatus &os, const QString &primer);
    static void setReversePrimer(U2OpStatus &os, const QString &primer);
    static void showPrimersDetails(U2OpStatus &os);

    // Utility
    static bool isSearchAlgorithmShowHideWidgetOpened(U2OpStatus &os);
    static bool isSearchInShowHideWidgetOpened(U2OpStatus &os);
    static bool isOtherSettingsShowHideWidgetOpened(U2OpStatus &os);
    static bool isSaveAnnotationToShowHideWidgetOpened(U2OpStatus &os);
    static bool isAnnotationParametersShowHideWidgetOpened(U2OpStatus &os);

    static void openSearchAlgorithmShowHideWidget(U2OpStatus &os, bool open = true);
    static void openSearchInShowHideWidget(U2OpStatus &os, bool open = true);
    static void openOtherSettingsShowHideWidget(U2OpStatus &os, bool open = true);
    static void openSaveAnnotationToShowHideWidget(U2OpStatus &os, bool open = true);
    static void openAnnotationParametersShowHideWidget(U2OpStatus &os, bool open = true);

private:
    static QMap<Tabs, QString> initNames();
    static QMap<Tabs, QString> initInnerWidgetNames();
};

}   // namespace U2

#endif // _U2_GT_UTILS_OPTION_PANEL_SEQUENCE_VIEW_H_
