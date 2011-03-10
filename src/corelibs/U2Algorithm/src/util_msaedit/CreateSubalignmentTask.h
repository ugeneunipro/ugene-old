#ifndef _U2_CREATE_SUBALIGNMENT_TASK_H_
#define _U2_CREATE_SUBALIGNMENT_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/GUrl.h>
#include <U2Core/U2Region.h>
#include <U2Core/MAlignmentObject.h>



namespace U2{

class U2ALGORITHM_EXPORT CreateSubalignmentTask : public Task {
    Q_OBJECT
public:    
    CreateSubalignmentTask(MAlignmentObject* _maObj, U2Region _window, 
                            const QStringList& _seqNames, const GUrl& _url, 
                            bool saveImmediately = false );

    ~CreateSubalignmentTask(){};

    void prepare();
    Task::ReportResult report();

signals:
    void documentCreated(Document *);

private:
    bool                saveToAnother;
    Document *          curDoc;
    MAlignmentObject*   maObj;
    U2Region             window;
    QStringList         seqNames;
    GUrl                url;
    Document*           newDoc;
    bool                saveImmediately;
};

}

#endif
