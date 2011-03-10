#ifndef _U2_WORKFLOW_FORMAT_H_
#define _U2_WORKFLOW_FORMAT_H_

#include "WorkflowDesignerPlugin.h"
#include <U2Core/DocumentModel.h>
#include <U2Core/GObject.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Gui/ObjectViewTasks.h>

#include <QtCore/QStringList>

#include <QtXml/qdom.h>
class QDomDocument;

namespace U2 {

class WorkflowView;

class WorkflowDocFormat : public DocumentFormat {
    Q_OBJECT
public:
    WorkflowDocFormat(QObject* p);
    
    static const DocumentFormatId FORMAT_ID;
    virtual DocumentFormatId getFormatId() const {return FORMAT_ID;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* createNewDocument(IOAdapterFactory* io, const QString& url, const QVariantMap& fs = QVariantMap());

    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    virtual void storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io);

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& url = GUrl()) const;

private:
    QString formatName;
};

class WorkflowGObject : public GObject {
    Q_OBJECT
public:
    static const GObjectType TYPE;
    WorkflowGObject(const QString& objectName, const QString& s, const QVariantMap& map = QVariantMap()) 
        : GObject(TYPE, objectName), serializedScene(s), view(NULL) { Q_UNUSED(map); }

    QString getSceneRawData() const {return serializedScene;}
    void setSceneRawData(const QString & data);
    virtual GObject* clone() const;
    virtual bool isTreeItemModified () const;
    void setView(WorkflowView* view);
    WorkflowView* getView() const {return view;}

protected:
    QString serializedScene;
    WorkflowView* view;
};

class WorkflowViewFactory : public GObjectViewFactory {
    Q_OBJECT
public:
    static const GObjectViewFactoryId ID;
    WorkflowViewFactory(QObject* p = NULL) : GObjectViewFactory(ID, U2::WorkflowDesignerPlugin::tr("Workflow Designer"), p) {}    

    virtual bool canCreateView(const MultiGSelection& multiSelection);
    virtual Task* createViewTask(const MultiGSelection& multiSelection, bool single = false);
};

class OpenWorkflowViewTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenWorkflowViewTask(Document* doc);
    virtual void open();
};

}//namespace

#endif
