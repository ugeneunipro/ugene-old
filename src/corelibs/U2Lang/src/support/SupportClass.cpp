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

#include "SupportClass.h"

namespace U2 {

/************************************************************************/
/* Problem */
/************************************************************************/
const QString Problem::U2_ERROR = "error";
const QString Problem::U2_WARNING = "warning";

Problem::Problem(const QString &_message, const QString &_actor, const QString& _type)
    : message(_message),
      actor(_actor),
      type(_type) {
}

bool Problem::operator== (const Problem &other) const {
    return (actor == other.actor) &&
            (message == other.message) &&
            (type == other.type);
}

}   // namespace U2
