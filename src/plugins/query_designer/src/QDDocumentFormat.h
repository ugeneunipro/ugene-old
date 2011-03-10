#ifndef _U2_QD_DOC_FORMAT_H_
#define _U2_QD_DOC_FORMAT_H_

#include <U2Core/DocumentModel.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Core/GObject.h>
#include <U2Gui/ObjectViewTasks.h>

#include "QueryDesignerPlugin.h"


namespace U2 {

class QDDocFormat : public DocumentFormat {
    Q_OBJECT
public:
    QDDocFormat(QObject* p);

    static const DocumentFormatId FORMAT_ID;
    virtual DocumentFormatId getFormatId() const {return FORMAT_ID;}

    virtual const QString& getFormatName() const {return formatName;}

    virtual Document* createNewDocument(IOAdapterFactory* io, const QString& url, const QVariantMap& fs = QVariantMap());

    virtual Document* loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode mode = DocumentLoadMode_Whole);

    virtual void storeDocument( Document* d, TaskStateInfo& ts, IOAdapter* io);

    virtual FormatDetectionResult checkRawData(const QByteArray& rawData, const GUrl& = GUrl()) const;

private:
    QString formatName;
};

class QueryScene;

class QDGObject : public GObject {
    Q_OBJECT
public:
    static const GObjectType TYPE;
    QDGObject(const QString& objectName, const QString& data, const QVariantMap& map = QVariantMap())
        : GObject(TYPE, objectName), serializedScene(data), scene(NULL) { Q_UNUSED(map); }

    virtual GObject* clone() const;
    QueryScene* getScene() const { return scene; }
    void setSceneRawData(const QString& d) { serializedScene=d; }
    QString getSceneRawData() const { return serializedScene; }
private:
    QString serializedScene;
    QueryScene* scene;
};

class QDViewFactory : public GObjectViewFactory {
    Q_OBJECT
public:
    static const GObjectViewFactoryId ID;
    QDViewFactory(QObject* p = NULL) : GObjectViewFactory(ID, U2::QueryDesignerPlugin::tr("Query Designer"), p) {}

    virtual bool canCreateView(const MultiGSelection& multiSelection);
    virtual Task* createViewTask(const MultiGSelection& multiSelection, bool single = false);
};

class OpenQDViewTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenQDViewTask(Document* doc);
    virtual void open();
};

}//namespace

#endif

