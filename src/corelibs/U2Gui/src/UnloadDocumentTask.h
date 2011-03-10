#ifndef _U2_UNLOAD_DOCUMENT_TASK_H_
#define _U2_UNLOAD_DOCUMENT_TASK_H_

#include <U2Core/GUrl.h>
#include <U2Core/Task.h>
#include <U2Core/UnloadedObject.h>

#include <QtCore/QPointer>

namespace U2 {

class Document;
class SaveDocumentTask;

enum UnloadDocumentTask_SaveMode {
    UnloadDocumentTask_SaveMode_Ask,
    UnloadDocumentTask_SaveMode_NotSave,
    UnloadDocumentTask_SaveMode_Save
};


class U2GUI_EXPORT UnloadDocumentTask: public Task {
    Q_OBJECT
public:
    UnloadDocumentTask(Document* doc, bool save);
    ReportResult report();

    static void runUnloadTaskHelper(const QList<Document*>& docs, UnloadDocumentTask_SaveMode sm);
    static QString checkSafeUnload(Document* d);

private:
    QPointer<Document>      doc;
    SaveDocumentTask*       saveTask;
};

}//namespace

#endif
