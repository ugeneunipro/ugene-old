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


#ifndef _U2_REMOTE_MACHINE_SCANNER_H_
#define _U2_REMOTE_MACHINE_SCANNER_H_

#include <QtCore/QList>

#include "RemoteMachine.h"

namespace U2 {

/*
 * Base class. Each transport protocol that can scan remote machines should implement this
 */
class U2REMOTE_EXPORT RemoteMachineScanner : public QObject {
public:
    RemoteMachineScanner( bool filter = true );
    virtual ~RemoteMachineScanner();
    
    virtual void startScan() = 0;
    /* creates new allocated list of machine settings. caller should delete them. returned machines - newly scanned since startScan() */
    virtual QList< RemoteMachineSettings * > takeScanned() = 0;
    
    virtual void setFilterLocalHost( bool filter );
    virtual bool getFilterLocalHost() const;
    
protected:
    bool filterLocalHost;
    
}; // RemoteMachineScanner

} // U2

#endif // _U2_REMOTE_MACHINE_SCANNER_H_
