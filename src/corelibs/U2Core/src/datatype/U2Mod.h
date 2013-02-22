/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MOD_H_
#define _U2_MOD_H_

#include <U2Core/U2Type.h>


namespace U2 {

/** Modification types */
class U2CORE_EXPORT U2ModType {
public:
    /** Object */
    static const qint64 objUpdatedName;

    /** MSA modification types */
    static const qint64 msaUpdatedAlphabet;
    static const qint64 msaAddedRows;
    static const qint64 msaAddedRow;
    static const qint64 msaRemovedRows;
    static const qint64 msaRemovedRow;
    static const qint64 msaUpdatedRowContent;
    static const qint64 msaUpdatedGapModel;
    static const qint64 msaSetNewRowsOrder;
    static const qint64 msaUpdatedRowName;

    static bool isObjectModType(qint64 modType) { return modType > 0 && modType < 999; }
    static bool isMsaModType(qint64 modType) { return modType >= 3000 && modType < 3100; }
};

/** Single modification of a dbi object */
class U2CORE_EXPORT U2ModStep {
public:
    /** ID of the modification in the database */
    qint64         id;

    /** ID of the dbi object */
    U2DataId       objectId;

    /** The object has been modified from 'version' to 'version + 1' */
    qint64         version;

    /** Type of the object modification */
    qint64         modType;

    /** Detailed description of the modification */
    QByteArray     details;
};

/** Several subsequent modification of a dbi object */
class U2CORE_EXPORT U2ModTrack {
public:
    QList<U2ModStep> mods;
};


} // namespace

#endif
