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

#ifndef _U2_REPLACE_PART_OF_SEQUENCE_TASK_H_
#define _U2_REPLACE_PART_OF_SEQUENCE_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2AnnotationUtils.h>

namespace U2 {

class Annotation;

class U2CORE_EXPORT ReplacePartOfSequenceTask : public Task {
    Q_OBJECT
public:
    ReplacePartOfSequenceTask(DocumentFormatId _dfId, DNASequenceObject *_seqObj, U2Region _regionToReplace, const DNASequence& newSeq, 
        U2AnnotationUtils::AnnotationStrategyForResize _str = U2AnnotationUtils::AnnotationStrategyForResize_Resize, 
        const GUrl& _url = GUrl(), bool _mergeAnnotations = false);
    virtual Task::ReportResult report();

private:
    void fixAnnotations();
    void preparationForSave();
    
    DocumentFormatId dfId;
    bool mergeAnnotations;
    Document *curDoc;
    Document *newDoc;
    bool save;
    GUrl url;
    U2AnnotationUtils::AnnotationStrategyForResize strat;
    QList<Document*> docs;
    DNASequenceObject *seqObj;
    QByteArray newSeq;
    U2Region regionToReplace;
};

} // U2

#endif //_U2_REPLACE_PART_OF_SEQUENCE_TASK_H_
