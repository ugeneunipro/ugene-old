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

#include <U2Core/U2OpStatus.h>
#include <U2Core/U2Type.h>


namespace U2 {

/** Modification types */
class U2CORE_EXPORT U2ModType {
public:
    /** Object */
    static const qint64 objUpdatedName;

    /** Sequence modification types*/
    static const qint64 sequenceUpdatedData;

    /** MSA modification types */
    static const qint64 msaUpdatedAlphabet;
    static const qint64 msaAddedRows;
    static const qint64 msaAddedRow;
    static const qint64 msaRemovedRows;
    static const qint64 msaRemovedRow;
    static const qint64 msaUpdatedRowContent;
    static const qint64 msaUpdatedGapModel;
    static const qint64 msaSetNewRowsOrder;

    static bool isObjectModType(qint64 modType) { return modType > 0 && modType < 999; }
    static bool isSequenceModType(qint64 modType) { return modType >= 1000 && modType < 1100; }
    static bool isMsaModType(qint64 modType) { return modType >= 3000 && modType < 3100; }
};

/** Single modification of a dbi object */
class U2CORE_EXPORT U2SingleModStep {
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

    /** ID of the multiple modifications step */
    qint64         multiStepId;
};

/**
 * Create an instance of this class when it is required to join
 * different modification into a one user action, i.e. all
 * these modifications will be undo/redo as a single action.
 * The user modifications step is finished when the object destructor is called.
 * Parameter "masterObjId" specifies ID of the object that initiated the changes.
 * Note that there might be other modified objects (e.g. child objects).
 *
 * WARNING!: you should limit the scope of the created instance to as small as possible,
 * as it "blocks" database!!
 */
class U2CORE_EXPORT U2UseCommonUserModStep {
public:
    U2UseCommonUserModStep(U2Dbi* _dbi, const U2DataId& _masterObjId, U2OpStatus& os);
    ~U2UseCommonUserModStep();
private:
    U2Dbi* dbi;
    bool valid;
};

} // namespace

#endif
