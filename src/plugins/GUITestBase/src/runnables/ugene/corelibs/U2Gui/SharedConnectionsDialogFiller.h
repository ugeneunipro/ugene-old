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

#ifndef _SHARED_CONNECTIONS_DIALOG_FILLER_H_
#define _SHARED_CONNECTIONS_DIALOG_FILLER_H_

#ifdef DELETE
#undef DELETE
#endif

#include <QtCore/QFlags>

#include "GTUtilsDialog.h"
#include "api/GTFileDialog.h"

namespace U2 {

class SharedConnectionsDialogFiller : public Filler {
public:
    enum Behavior { SAFE, UNSAFE };

    class Action {
    public:
        enum Type {ADD, CLICK, EDIT, DELETE, CONNECT, DISCONNECT, CANCEL};

        Action(Type type, QString itemName = "");
        Type type;
        QString itemName;
        QString dbName;

        // for CONNECT
        enum ConnectResult {OK, WRONG_DATA, INITIALIZE, DONT_INITIALIZE, VERSION, LOGIN};
        ConnectResult expectedResult;
    };

    SharedConnectionsDialogFiller(U2OpStatus &os, const QList<Action> &actions, const QFlags<Behavior> &behavior = QFlags<Behavior>(SAFE));
    void run();

private:
    QList<Action> actions;
    QFlags<Behavior> behavior;
};

} // U2

#endif // _SHARED_CONNECTIONS_DIALOG_FILLER_H_
