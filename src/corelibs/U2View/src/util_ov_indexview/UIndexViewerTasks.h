
#ifndef _U2_UINDEX_VIEWER_TASKS_H_
#define _U2_UINDEX_VIEWER_TASKS_H_

#include <U2Gui/ObjectViewTasks.h>
#include <U2Core/UIndexObject.h>
#include <U2Core/UnloadedObject.h>

namespace U2 {

class UIndexViewer;

class OpenUIndexViewerTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenUIndexViewerTask( UIndexObject* indObj );
    OpenUIndexViewerTask( Document* doc );
    OpenUIndexViewerTask( UnloadedObject* obj );
    
    virtual void open();
    
private:
    QPointer< UIndexObject > uindObj;
    GObjectReference         unloadedReference;
    
};

} // U2

#endif // _U2_UINDEX_VIEWER_TASKS_H_
