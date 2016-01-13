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

#ifndef _U2_FIND_PATTERN_SAVABLE_TAB_H_
#define _U2_FIND_PATTERN_SAVABLE_TAB_H_

#include <QtCore/QStringList>

#include <U2Gui/U2SavableWidget.h>

namespace U2 {

class U2SequenceObject;

class FindPatternWidgetSavableTab : public U2SavableWidget {
public:
    FindPatternWidgetSavableTab(QWidget *wrappedWidget, MWMDIWindow *contextWindow);
    ~FindPatternWidgetSavableTab();

    void    setChildValue(const QString &childId, const QVariant &value);
    void    setRegionWidgetIds(const QStringList &s);
private:
    QStringList regionWidgetIds;
};

} // namespace U2

#endif
