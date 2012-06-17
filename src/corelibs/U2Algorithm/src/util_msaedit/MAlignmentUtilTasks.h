/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_MALIGNMENT_UTIL_TASKS
#define _U2_MALIGNMENT_UTIL_TASKS

#include <U2Core/global.h>
#include <U2Core/Task.h>
#include <U2Core/MAlignmentObject.h>

namespace U2 {

class DNATranslation;

/**
 Performs in-place translation of malignment object
*/

class U2ALGORITHM_EXPORT TranslateMSA2AminoTask : public Task {
    Q_OBJECT
public:
    TranslateMSA2AminoTask(MAlignmentObject* obj);
    TranslateMSA2AminoTask(MAlignmentObject* obj, const QString& trId );
    const MAlignment& getTaskResult() { return resultMA; }
    void run();
    ReportResult report();
private:
    MAlignment resultMA;
    MAlignmentObject* maObj;
    DNATranslation* translation;
};


/**
 Wrapper for multiple alignment task
*/

class U2ALGORITHM_EXPORT AlignGObjectTask : public Task {
    Q_OBJECT
public:
    AlignGObjectTask(const QString& taskName, TaskFlags f, MAlignmentObject* maobj)
        : Task(taskName, f), obj(maobj) {}
    void setMAObject(MAlignmentObject* maobj) { obj = maobj; }
    MAlignmentObject* getMAObject() { return obj; }
protected:
    QPointer<MAlignmentObject> obj;
};


/**
 Multi task converts alignment object to amino representation if possible.
 This allows to: 
 1) speed up alignment 
 2) avoid errors of inserting gaps within codon boundaries  
*/

class U2ALGORITHM_EXPORT AlignInAminoFormTask : public Task {
    Q_OBJECT
public:
    AlignInAminoFormTask(MAlignmentObject* obj, AlignGObjectTask* alignTask, const QString& traslId);
    ~AlignInAminoFormTask();

    virtual void prepare();
    virtual void run();
    virtual ReportResult report();
protected:
    AlignGObjectTask* alignTask;
    MAlignmentObject *maObj, *clonedObj;
    MAlignment bufMA;
    QString traslId;
};


} // U2

#endif // _U2_MALIGNMENT_UTIL_TASKS
