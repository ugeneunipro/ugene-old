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


#ifndef _U2_GT_UTILS_START_PAGE_
#define _U2_GT_UTILS_START_PAGE_

#include "api/GTGlobals.h"

class QWebView;
class QWebElement;

namespace U2 {

class GTUtilsStartPage
{
public:
    enum Buttons{OpenFile, CreateSequence, CreateWorkflow, QuickStartGuide};
    static void openStartPage(U2OpStatus &os);
    static QWebView* getStartPage(U2OpStatus &os);
    static QWebElement getButton(U2OpStatus &os, Buttons button);
    static void clickButton(U2OpStatus &os, Buttons button);
    static void clickResentDocument(U2OpStatus &os, QString text);
    static void clickResentProject(U2OpStatus &os, QString text);
};

}

#endif // _U2_GT_UTILS_START_PAGE_
