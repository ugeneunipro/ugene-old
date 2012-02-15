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

#include "AnnotationData.h"

namespace U2 {

QDataStream& operator>>(QDataStream& dataStream, TriState& state) {
    int st;
    dataStream >> st;
    switch (st) {
        case 0: state = TriState_Yes; break;
        case 1: state = TriState_No; break;
        default: state = TriState_Unknown;
    }
    return dataStream;
}

QDataStream& operator<<(QDataStream& dataStream, const TriState& state) {
    switch (state) {
        case TriState_Yes: return dataStream << 0;
        case TriState_No: return dataStream << 1;
        default: return dataStream << 2;
    }
}

QDataStream& operator>>(QDataStream& dataStream, U2Qualifier& q) {
    return dataStream >> q.name >> q.value;
}

QDataStream& operator<<(QDataStream& dataStream, const U2Qualifier& q) {
    return dataStream << q.name << q.value;
}

QDataStream& operator>>(QDataStream& dataStream, AnnotationData& ) {
    assert(0);
    return dataStream; 
//    return dataStream >> data.name >> data.location >> data.qualifiers;
}

QDataStream& operator<<(QDataStream& dataStream, const AnnotationData& ) {
    assert(0);
    return dataStream; 
//    return dataStream << data.name << data.location << data.qualifiers;
}

} //namespace
