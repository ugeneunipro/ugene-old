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

#ifndef _GB2_HMMER3_UHMM_OBJECT_H_
#define _GB2_HMMER3_UHMM_OBJECT_H_

#include <QtCore/QString>

#include <U2Core/global.h>
#include <U2Core/GObject.h>

#include <hmmer3/hmmer.h>

namespace U2 {

class UHMMObject : public GObject {
    Q_OBJECT
public:
    static const QString        OT_ID;
    static const QString        OT_NAME;
    static const QString        OT_PNAME;
    static const QString        OT_SIGN;
    static const QString        OT_ICON;
    static const GObjectType    UHMM_OT;
    
    UHMMObject( P7_HMM* hmm, const QString& name );
    ~UHMMObject();
    
    const P7_HMM* getHMM() const;
    P7_HMM* takeHMM();
    
    virtual GObject* clone(const U2DbiRef& ref, U2OpStatus& os) const;
    
private:
    P7_HMM* hmm;
    
}; // UHMMObject

} // U2

#endif // _GB2_HMMER3_UHMM_OBJECT_H_
