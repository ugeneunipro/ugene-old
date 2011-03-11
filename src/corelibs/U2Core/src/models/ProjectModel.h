#ifndef _U2_PROJECT_MODEL_H_
#define _U2_PROJECT_MODEL_H_

#include "DocumentModel.h"
#include "StateLockableDataModel.h"

#define CURRENT_PROJECT_VERSION "1.0"

namespace U2 {

class GObjectViewState;
class Document;
class Project;
class Task;

#define PROJECT_FILE_PURE_EXT QString("uprj")
#define PROJECTFILE_EXT QString("." + PROJECT_FILE_PURE_EXT)

/// Service responsible for project loading / unloading
class U2CORE_EXPORT ProjectLoader  : public QObject {
public:
    virtual Task* openProjectTask(const QString& file, bool closeActiveProject) = 0;
    virtual Task* openProjectTask(const QList<GUrl>& urls, bool closeActiveProject) = 0;
    virtual Project* createProject(const QString& name, const QString& url, QList<Document*>& documents, QList<GObjectViewState*>& states) = 0;
};

/// Project model
class U2CORE_EXPORT Project : public StateLockableTreeItem {
    Q_OBJECT
    Q_PROPERTY( QString name WRITE setProjectName READ getProjectName )
    Q_PROPERTY( QString url WRITE setProjectURL READ getProjectURL )
    Q_PROPERTY( QList<Document*> docs READ getDocuments )

public:
    virtual const QString& getProjectName() const = 0;

    virtual void setProjectName(const QString& name) = 0;

    virtual const QString& getProjectURL() const = 0;

    virtual void setProjectURL(const QString&) = 0;

    virtual const QList<Document*>& getDocuments() const = 0;

    Q_INVOKABLE virtual void addDocument(Document* d) = 0;

    Q_INVOKABLE virtual void removeDocument(Document* d, bool autodelete = true) = 0;

    Q_INVOKABLE virtual Document* findDocumentByURL(const QString& url) const  = 0;

    Q_INVOKABLE virtual Document* findDocumentByURL(const GUrl& url) const  = 0;

    virtual bool lockResoureces(int sizeMB, const QString & url, QString &error) = 0;

    virtual const QList<GObjectViewState*>& getGObjectViewStates() const =0;

    virtual void addGObjectViewState(GObjectViewState* s) = 0;

    virtual void removeGObjectViewState(GObjectViewState* s) = 0;

    virtual void makeClean() = 0;

    static void setupToEngine(QScriptEngine *engine);
private:
    static QScriptValue toScriptValue(QScriptEngine *engine, Project* const &in);
    static void fromScriptValue(const QScriptValue &object, Project* &out);
signals:
    void si_projectURLChanged(const QString& oldURL);

    void si_projectRenamed(Project* p);

    void si_documentAdded(Document* d);

    void si_documentRemoved(Document* d);

    void si_objectViewStateAdded(GObjectViewState*);

    void si_objectViewStateRemoved(GObjectViewState*);
};

class DocumentSerialState {

};

}//namespace
Q_DECLARE_METATYPE(U2::Project*)
Q_DECLARE_METATYPE(QList<U2::Document*>)

#endif
