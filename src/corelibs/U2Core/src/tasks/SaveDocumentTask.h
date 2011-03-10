#ifndef _U2_SAVE_DOCUMENT_TASK_H_
#define _U2_SAVE_DOCUMENT_TASK_H_

#include <U2Core/GUrl.h>
#include <U2Core/Task.h>
#include <U2Core/UnloadedObject.h>

#include <QtCore/QPointer>

namespace U2 {

class Document;
class StateLock;
class IOAdapterFactory;
class DocumentFormat;

enum SaveDocFlag {
    SaveDoc_Overwrite = 0x0,
    SaveDoc_Append = 0x1, 
    SaveDoc_Roll = 0x2, 
    SaveDoc_DestroyAfter = 0x4
};

typedef QFlags<SaveDocFlag>  SaveDocFlags;

class U2CORE_EXPORT SaveDocumentTask : public Task {
    Q_OBJECT
public:
    SaveDocumentTask(Document* doc, IOAdapterFactory* iof = NULL, const GUrl& url = GUrl());
    SaveDocumentTask(Document* doc, SaveDocFlags flags, const QSet<QString>& excludeFileNames = QSet<QString>());

    virtual void prepare();

    virtual void run();

    ReportResult report();

    const GUrl& getURL() const {return url;}

    IOAdapterFactory* getIOAdapterFactory() const {return iof;}

    const QPointer<Document>& getDocument() const {return doc;}

    //used in file-name rolling mode
    void setExcludeFileNames(const QSet<QString>& _excludeFileNames) {excludeFileNames = _excludeFileNames;}

private:
    StateLock*          lock;
    QPointer<Document>  doc;
    IOAdapterFactory*   iof;
    GUrl                url;
    SaveDocFlags        flags;
    QSet<QString>       excludeFileNames;
};


class U2CORE_EXPORT SaveMiltipleDocuments: public Task {
    Q_OBJECT
public:

    SaveMiltipleDocuments(const QList<Document*>& docs, bool askBeforeSave);

    static QList<Document*> findModifiedDocuments(const QList<Document*>& docs);
};

class U2CORE_EXPORT SaveCopyAndAddToProjectTask : public Task {
    Q_OBJECT
public:
    SaveCopyAndAddToProjectTask(Document* doc, IOAdapterFactory* iof, const GUrl& url);
    ReportResult report();
private:
    SaveDocumentTask*           saveTask;
    QList<UnloadedObjectInfo>   info;
    GUrl                        url;
    GUrl                        origURL;
    DocumentFormat*             df;
    QVariantMap                 hints;
};


class U2CORE_EXPORT RelocateDocumentTask : public Task {
    Q_OBJECT
public:
    RelocateDocumentTask(const GUrl& fromURL, const GUrl& toURL);
    ReportResult report();

public:
    GUrl fromURL;
    GUrl toURL;
};

}//namespace

#endif
