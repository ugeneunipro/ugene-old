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

#ifndef _U2_OPSTATUS_UTILS_H_
#define _U2_OPSTATUS_UTILS_H_

#include <U2Core/U2OpStatus.h>
#include <U2Core/Task.h>

namespace U2 {

/** 
    Default helper stub for U2OpStatus 
    Note: implementation is not thread safe!
*/
class U2CORE_EXPORT U2OpStatusImpl : public U2OpStatus {
public:
    U2OpStatusImpl() : cancelFlag(false), progress(-1){}

    virtual void setError(const QString & err) {error = err;}
    virtual QString getError() const  {return error;}

    virtual bool hasError() const {return !error.isEmpty();}

    virtual bool isCanceled() const {return cancelFlag != 0;}
    virtual void setCanceled(bool v)  {cancelFlag = v;}

    virtual int getProgress() const {return progress;}
    virtual void setProgress(int v)  {progress = v;}

    virtual QString getStatusDesc() const {return statusDesc;}
    virtual void setStatusDesc(const QString& desc)  {statusDesc = desc;}

private:
    QString error;
    int     cancelFlag;
    int     progress;
    QString statusDesc;
};

/** Default helper stub for U2OpStatus */
class U2CORE_EXPORT U2OpTaskStatus: public U2OpStatus {
public:
    U2OpTaskStatus(TaskStateInfo* i) : ti(i)  {}
    virtual void setError(const QString & err)  {ti->setError(err);}
    virtual QString getError() const  {return ti->getError();}
    virtual bool hasError() const {return ti->hasErrors();}

    virtual bool isCanceled() const {return ti->cancelFlag != 0;}
    virtual void setCanceled(bool v)  {ti->cancelFlag = v;};

    virtual int getProgress() const  {return ti->progress;}
    virtual void setProgress(int v)  {ti->progress = v;}

    virtual QString getStatusDesc() const {return ti->getStateDesc();}
    virtual void setStatusDesc(const QString& desc)  {ti->setStateDesc(desc);}

private:
    TaskStateInfo* ti;
};


}// namespace

#endif
