/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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


#ifndef GTUTILSNOTIFICATIONS_H
#define GTUTILSNOTIFICATIONS_H

#include "GTUtilsDialog.h"
#include "api/GTGlobals.h"

namespace U2 {

class NotificationChecker : public QObject {
    Q_OBJECT
public:
    NotificationChecker(U2OpStatus &_os);
public slots:
    void sl_checkNotification();
private:
    QTimer* t;
    U2OpStatus &os;
};

class NotificationDialogFiller : public Filler{
public:
    NotificationDialogFiller(U2OpStatus &os, QString _message = ""):
        Filler(os, "NotificationDialog"), message(_message){}
    virtual void run();
private:
    QString message;
};

class GTUtilsNotifications
{
public:
    static void waitForNotification(U2OpStatus &os, bool dialogExpected = true);
};
}

#endif // GTUTILSNOTIFICATIONS_H
