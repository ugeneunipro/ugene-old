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

#ifndef _U2_EXPORT_PLUGIN_TASKS_H_
#define _U2_EXPORT_PLUGIN_TASKS_H_

#include <U2Core/BaseDocumentFormats.h>

#include <U2Core/Task.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/MAlignment.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DocumentProviderTask.h>


namespace U2 {


/** A task to adds exported document to project and open view*/
//TODO: make this task a general purpose routine
class AddExportedDocumentAndOpenViewTask: public Task {
    Q_OBJECT
public:
    AddExportedDocumentAndOpenViewTask(DocumentProviderTask* t);
    QList<Task*> onSubTaskFinished( Task* subTask );
private:
    DocumentProviderTask* exportTask;
};


/** A task to save alignment to CLUSTAL */
class ExportAlignmentTask : public DocumentProviderTask  {
    Q_OBJECT
public:
    ExportAlignmentTask(const MAlignment& ma, const QString& fileName, DocumentFormatId f);

    void run();

private:
    MAlignment              ma;
    QString                 fileName;
    DocumentFormatId        format;
};


/** A task to export alignment to FASTA */
class ExportMSA2SequencesTask : public DocumentProviderTask {
    Q_OBJECT
public:
    ExportMSA2SequencesTask(const MAlignment& ma, const QString& url, bool trimAli, DocumentFormatId format);

    void run();

private:
    MAlignment              ma;
    QString                 url;
    bool                    trimAli;
    QString                 format;
};

class ExportMSA2MSATask : public DocumentProviderTask {
    Q_OBJECT
public:
    ExportMSA2MSATask(const MAlignment& ma, int offset, int len, const QString& url, 
        const QList<DNATranslation*>& aminoTranslations, DocumentFormatId format);

    void run();

private:
    MAlignment              ma;
    int                     offset;
    int                     len;
    QString                 url;
    QString                 format;
    QList<DNATranslation*>  aminoTranslations; // amino translation for a sequences in alignment. If not NULL -> sequence is translated
};

class DNAChromatogramObject;
class LoadDocumentTask;

/** A task to export chromatogram to SCF */

struct ExportChromatogramTaskSettings {
    ExportChromatogramTaskSettings() : reverse(false), complement(false), loadDocument(false) {}
    QString url;
    bool reverse;
    bool complement;
    bool loadDocument;
};


class ExportDNAChromatogramTask : public DocumentProviderTask {
    Q_OBJECT
public:
    ExportDNAChromatogramTask(DNAChromatogramObject* chromaObj, const ExportChromatogramTaskSettings& url);
    void prepare();
    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    DNAChromatogramObject*  cObj;
    ExportChromatogramTaskSettings settings;
    LoadDocumentTask* loadTask;
};




}//namespace

#endif
