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

#ifndef _U2_EXPORT_SEQUENCE_PLUGIN_TASKS_H_
#define _U2_EXPORT_SEQUENCE_PLUGIN_TASKS_H_

#include "ExportTasks.h"

namespace U2 {

class DNAAlphabet;
class DNATranslation;

/** An item to export sequence. Contains unprocessed sequence + annotations + info about required transformations */
class ExportSequenceItem {
public:
    ExportSequenceItem() : complTT(NULL), aminoTT(NULL), backTT(NULL){}
    DNASequence                 sequence;   // the sequences to copy
    QList<SharedAnnotationData> annotations;//annotations to copy
    DNATranslation*             complTT;    // complement translations for a sequence. Used only if 'strand' is 'compl' or 'both'
    DNATranslation*             aminoTT;    // amino translation for a sequence. If not NULL -> sequence is translated
    DNATranslation*             backTT;     // nucleic translation for a sequence. If not NULL -> sequence is back translated
};

class ExportSequenceTaskSettings {
public:
    ExportSequenceTaskSettings() {
        merge = false;
        mergeGap = 0;
        strand = TriState_Yes;
        allAminoFrames = false;
        formatId = BaseDocumentFormats::PLAIN_FASTA;
        mostProbable = true;
        saveAnnotations = false;
    }

    QList<ExportSequenceItem> items;            // sequences to export
    
    QString             fileName;               // result file name
    bool                merge;                  // if true -> multiple sequences are merged
    int                 mergeGap;               // the gap between sequences if merged

    TriState            strand;                 // Yes -> direct, No -> complement, Unknown -> Both
    bool                allAminoFrames;         // for every sequence and (every complement if needed) generates 3-frame amino translations

    bool                mostProbable;           // True - use most probable codon, False - use distribution according to frequency

    bool                saveAnnotations;        // Store available annotations for sequences into result file too

    DocumentFormatId    formatId;
};

/** Exports sequences a file */
class ExportSequenceTask: public AbstractExportTask {
    Q_OBJECT
public:
    ExportSequenceTask(const ExportSequenceTaskSettings& s);
    
    void run();
    
    virtual Document* getDocument() const {return doc.get();}

private:
    std::auto_ptr<Document> doc; 
    ExportSequenceTaskSettings config;
};

//////////////////////////////////////////////////////////////////////////
// Task to export sequences under annotations

class ExportSequenceAItem {
public:
    ExportSequenceAItem() : aminoTT(NULL), complTT(NULL) {}

    DNASequence                 sequence;     // sequence
    QList<SharedAnnotationData> annotations;  // annotated regions to be exported
    DNATranslation*             aminoTT;      // if not null -> sequence regions will be translated (0-frame only)
    DNATranslation*             complTT;      // if not null & annotation location is on complement strand - it will be rev-complemented

};


class ExportAnnotationSequenceTaskSettings {
public:
    QList<ExportSequenceAItem>  items;                  // data to export
    ExportSequenceTaskSettings exportSequenceSettings; // extra configuration for ExportSequenceTask
};

class ExportAnnotationSequenceSubTask : public Task{
Q_OBJECT
public:
    ExportAnnotationSequenceSubTask(ExportAnnotationSequenceTaskSettings& s);
    void run();
private:
    ExportAnnotationSequenceTaskSettings& config;
};

class ExportAnnotationSequenceTask : public AbstractExportTask {
    Q_OBJECT
public:
    ExportAnnotationSequenceTask(const ExportAnnotationSequenceTaskSettings& s);

    virtual Document* getDocument() const {return exportSubTask->getDocument();}
    virtual QList<Task*> onSubTaskFinished(Task* subTask);

private:
    ExportAnnotationSequenceTaskSettings    config;
    ExportAnnotationSequenceSubTask*        extractSubTask;
    ExportSequenceTask*                     exportSubTask;
};

}//namespace

#endif
