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


#ifndef _U2_PROTOCOL_UI_H_
#define _U2_PROTOCOL_UI_H_

#include <cassert>

#include <QtCore/QList>
#include <QtGui/QWidget>
#include "RemoteMachine.h"

namespace U2 {

/*
 * Base class for GUI form with remote machine info
 */
class U2REMOTE_EXPORT ProtocolUI : public QWidget {
public:
    ProtocolUI();
    virtual ~ProtocolUI();
    
    // after user clicks 'ok', creates remote machine settings based on the info in the form. Caller should delete it
    virtual RemoteMachineSettingsPtr createMachine() const = 0;
    virtual void initializeWidget( const RemoteMachineSettingsPtr& settings ) = 0;
    virtual void clearWidget() = 0;
    virtual QString validate() const = 0;
    virtual QDialog* createUserTasksDialog(const RemoteMachineSettingsPtr& settings, QWidget* parent) = 0;
    
private:
    ProtocolUI( const ProtocolUI & );
    ProtocolUI & operator=( const ProtocolUI & );
    
}; // ProtocolUI



} // U2

#endif // _U2_PROTOCOL_UI_H
