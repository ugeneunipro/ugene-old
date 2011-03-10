#ifndef _U2_EXPORT_PLUGIN_TASKS_H_
#define _U2_EXPORT_PLUGIN_TASKS_H_

#include <U2Core/BaseDocumentFormats.h>

#include <U2Core/Task.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/MAlignment.h>
#include <memory>


namespace U2 {

/** A task to save alignment to CLUSTAL */
class SaveAlignmentTask : public Task {
    Q_OBJECT
public:
    SaveAlignmentTask(const MAlignment& ma, const QString& fileName, DocumentFormatId f);

    void run();

    virtual Document* getDocument() const {return doc.get();}

private:
    MAlignment              ma;
    QString                 fileName;
    DocumentFormatId        format;
    std::auto_ptr<Document> doc;
};


/** A task to export alignment to FASTA */
class SaveMSA2SequencesTask : public Task {
    Q_OBJECT
public:
    SaveMSA2SequencesTask(const MAlignment& ma, const QString& url, bool trimAli, DocumentFormatId format);

    void run();

    virtual Document* getDocument() const {return doc.get();}

private:
    MAlignment              ma;
    QString                 url;
    bool                    trimAli;
    QString                 format;
    std::auto_ptr<Document> doc;
};

}//namespace

#endif
