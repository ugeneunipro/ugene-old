#ifndef _U2_EXPORT_PLUGIN_TASKS_H_
#define _U2_EXPORT_PLUGIN_TASKS_H_

#include <U2Core/BaseDocumentFormats.h>

#include <U2Core/Task.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/MAlignment.h>
#include <U2Core/AnnotationTableObject.h>
#include <memory>


namespace U2 {

/** Base class for all export tasks */
class AbstractExportTask : public Task {
public:
    AbstractExportTask(const QString& _name, TaskFlags f) : Task(_name, f){}
    virtual Document* getDocument() const = 0;
};

/** A task to adds exported document to project and open view*/
//TODO: make this task a general purpose routine
class AddDocumentAndOpenViewTask: public Task {
    Q_OBJECT
public:
    AddDocumentAndOpenViewTask(AbstractExportTask* t);
    QList<Task*> onSubTaskFinished( Task* subTask );
private:
    AbstractExportTask* exportTask;
};


/** A task to save alignment to CLUSTAL */
class ExportAlignmentTask : public AbstractExportTask  {
    Q_OBJECT
public:
    ExportAlignmentTask(const MAlignment& ma, const QString& fileName, DocumentFormatId f);

    void run();

    virtual Document* getDocument() const {return doc.get();}

private:
    MAlignment              ma;
    QString                 fileName;
    DocumentFormatId        format;
    std::auto_ptr<Document> doc;
};


/** A task to export alignment to FASTA */
class ExportMSA2SequencesTask : public AbstractExportTask {
    Q_OBJECT
public:
    ExportMSA2SequencesTask(const MAlignment& ma, const QString& url, bool trimAli, DocumentFormatId format);

    void run();

    virtual Document* getDocument() const {return doc.get();}

private:
    MAlignment              ma;
    QString                 url;
    bool                    trimAli;
    QString                 format;
    std::auto_ptr<Document> doc;
};

class ExportMSA2MSATask : public AbstractExportTask {
    Q_OBJECT
public:
    ExportMSA2MSATask(const MAlignment& ma, int offset, int len, const QString& url, 
        const QList<DNATranslation*>& aminoTranslations, DocumentFormatId format);

    void run();

    virtual Document* getDocument() const {return doc.get();}

private:
    MAlignment              ma;
    int                     offset;
    int                     len;
    QString                 url;
    QString                 format;
    QList<DNATranslation*>  aminoTranslations; // amino translation for a sequences in alignment. If not NULL -> sequence is translated
    std::auto_ptr<Document> doc;
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


class ExportDNAChromatogramTask : public AbstractExportTask {
    Q_OBJECT
public:
    ExportDNAChromatogramTask(DNAChromatogramObject* chromaObj, const ExportChromatogramTaskSettings& url);
    void prepare();
    virtual Document* getDocument() const;
private:
    DNAChromatogramObject*  cObj;
    ExportChromatogramTaskSettings settings;
    LoadDocumentTask* loadTask;
};




}//namespace

#endif
