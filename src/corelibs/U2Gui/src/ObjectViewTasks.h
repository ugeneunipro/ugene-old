#ifndef _U2_OBJECT_VIEW_TASKS_H_
#define _U2_OBJECT_VIEW_TASKS_H_

#include <U2Core/Task.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/GObjectReference.h>

#include <QtCore/QPointer>

namespace U2 {

class Document;
class GObject;

class U2GUI_EXPORT ObjectViewTask : public Task {
    Q_OBJECT
public:
    enum Type {
        Type_Open,
        Type_Update
    };

    ObjectViewTask(GObjectView* view, const QString& stateName, const QVariantMap& s = QVariantMap());

    ObjectViewTask(GObjectViewFactoryId fid, const QString& viewName = QString(), const QVariantMap& s = QVariantMap());

    virtual void prepare();
    virtual ReportResult report();


    virtual void open(){};
    virtual void update(){};

    virtual void onDocumentLoaded(Document* d){ Q_UNUSED(d); }

protected:
    Type                    taskType;
    QVariantMap             stateData;
    QPointer<GObjectView>   view;
    QString                 viewName;
    bool                    stateIsIllegal;
    QStringList             objectsNotFound;
    QStringList             documentsNotFound;


    QList<QPointer<Document> > documentsToLoad;
    QList<QPointer<Document> > documentsFailedToLoad;

    QList<QPointer<GObject> > selectedObjects;
};

class U2GUI_EXPORT AddToViewTask : public Task {
    Q_OBJECT
public:
    AddToViewTask(GObjectView* v, GObject* obj);
    ReportResult report();

    QPointer<GObjectView>   objView;
    QString                 viewName;
    GObjectReference        objRef;
    QPointer<Document>      objDoc;
    
};

} // namespace


#endif
