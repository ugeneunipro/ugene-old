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


#ifndef _U2_GUI_TEST_OP_STATUS_H_
#define _U2_GUI_TEST_OP_STATUS_H_

#include <U2Core/U2OpStatus.h>

namespace U2 {
class GUITestOpStatus : public U2OpStatus
{
public:
    GUITestOpStatus(){}

    virtual void setError(const QString & err){error = err; throw  this;}
    virtual QString getError() const {return error;}
    virtual bool hasError() const {return !error.isEmpty();}

    virtual bool isCanceled() const {return false;}
    virtual void setCanceled(bool v) {Q_UNUSED(v)}

    virtual int getProgress() const {return 100;}
    virtual void setProgress(int v) {Q_UNUSED(v)}

    virtual QString getDescription() const {return "";}
    virtual void setDescription(const QString& desc) {}
private:
    QString error;
};
}
#endif // GUITESTOPSTATUS_H
