/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_TEXT_SELECTION_H_
#define _U2_TEXT_SELECTION_H_

#include "LRegionsSelection.h"
#include "SelectionTypes.h"

namespace U2 {

class TextObject;

class  U2CORE_EXPORT TextSelection : public LRegionsSelection {
public:
    TextSelection(TextObject* _obj, QObject* p = NULL) : LRegionsSelection(GSelectionTypes::TEXT, p), obj(_obj) {}

private:
    TextObject* obj;
};

}//namespace

#endif
