/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GUI_GTACTION_H_
#define _U2_GUI_GTACTION_H_

#include "api/GTGlobals.h"

class QAbstractButton;
namespace U2 {

class GTAction {
public:
    // returns first QAbstractButton associated with an action with a given name
    static QAbstractButton* button(U2OpStatus &os, const QString &actionName, QObject *parent = NULL);

    static QAction* findAction(U2OpStatus &os, const QString &actionName, QObject *parent = NULL);
};

} //namespace

#endif
