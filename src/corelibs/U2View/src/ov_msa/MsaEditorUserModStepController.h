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

#ifndef _U2_MSA_EDITOR_USER_MOD_STEP_CONTROLLER_H_
#define _U2_MSA_EDITOR_USER_MOD_STEP_CONTROLLER_H_

namespace U2 {

class U2UseCommonUserModStep;
class U2EntityRef;
class U2OpStatus;

class MsaEditorUserModStepController
{
public:
    MsaEditorUserModStepController( const U2EntityRef &msaEntityRef );
    ~MsaEditorUserModStepController( );

    void startTracking( U2OpStatus &os );
    void finishTracking( );

private:
    const U2EntityRef &msaEntityRef;
    U2UseCommonUserModStep *msaChangeTracker;
};

} // namespace U2

#endif // _U2_MSA_EDITOR_USER_MOD_STEP_CONTROLLER_H_
