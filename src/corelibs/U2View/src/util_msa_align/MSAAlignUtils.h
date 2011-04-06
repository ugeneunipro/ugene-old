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

#ifndef _U2_MSA_ALIGN_UTILS_H_
#define _U2_MSA_ALIGN_UTILS_H_

#include <U2Core/global.h>
#include <U2Core/GUrl.h>
#include <U2Core/Task.h>

#include <U2Algorithm/MSAAlignTask.h>

namespace U2 {

class MSAAlignTaskSettings;
class Document;
class LoadDocumentTask;
class SaveDocumentTask;
class AddDocumentTask;
class MAlignmentObject;
class DNATranslation;
class StateLock;

/**
 Performs in-place translation of malignment object
*/

class TranslateMSA2AminoTask : public Task {
    Q_OBJECT
public:
    TranslateMSA2AminoTask(MAlignmentObject* obj );
    const MAlignment& getTaskResult() { return resultMA; }
    void run();
    ReportResult report();
private:
    MAlignment resultMA;
    MAlignmentObject* maObj;
    QList<DNATranslation*> translations;
};

class U2VIEW_EXPORT MAlignmentGObjectTask : public Task {
    Q_OBJECT
public:
    MAlignmentGObjectTask(const QString& taskName, TaskFlags f, MAlignmentObject* maobj)
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

class U2VIEW_EXPORT MSAAlignMultiTask : public Task {
    Q_OBJECT
public:
    MSAAlignMultiTask(MAlignmentObject* obj, MAlignmentGObjectTask* alignTask, bool convertToAmino);
    virtual void prepare();
    virtual void run();
    virtual ReportResult report();
protected:
    MAlignmentGObjectTask* alignTask;
    MAlignmentObject *maObj, *clonedObj;
    bool convertToAmino;
    MAlignment bufMA;

};


} // U2

#endif // _U2_MSA_ALIGN_TASK_H_
