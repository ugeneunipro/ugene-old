#ifndef _U2_PROJECT_IMPL_H_
#define _U2_PROJECT_IMPL_H_

#include <U2Core/ProjectModel.h>
#include <U2Core/AppResources.h>

#include <QtCore/QMap>


namespace U2 {

class FailedToLoadFormat;
class ServiceLock;
class SaveProjectTask;
class LoadProjectTask;

class ProjectImpl : public Project {
    Q_OBJECT
	friend class SaveProjectTask;
	friend class LoadProjectTask;
public:
    ProjectImpl(const QString& _name, const QString& _url, 
                const QList<Document*>& _docs = QList<Document*>(), 
                const QList<GObjectViewState*>& _states = QList<GObjectViewState*>());
	~ProjectImpl();
    
	virtual const QString& getProjectName() const {return name;}

	virtual void setProjectName(const QString& name);

	virtual const QString& getProjectURL() const {return url;}

    virtual void setProjectURL(const QString&);

	virtual const QList<Document*>& getDocuments() const {return docs;}

	virtual void addDocument(Document* d);

	virtual void removeDocument(Document* d, bool autodelete = true);

    virtual bool lockResoureces(int sizeMB, const QString & url, QString &error);

	virtual Document* findDocumentByURL(const QString& url) const;
	
	virtual Document* findDocumentByURL(const GUrl& url) const { 
		return findDocumentByURL(url.getURLString()); 
	}
	
	virtual const QList<GObjectViewState*>& getGObjectViewStates() const {return objectViews;}

	virtual void addGObjectViewState(GObjectViewState* s);

	virtual void removeGObjectViewState(GObjectViewState* s);

	virtual void makeClean();

private slots:
	void sl_onStateModified(GObjectViewState*);

private:
	void addState(GObjectViewState* s);
    
    QString name;
    QString url;
	QList<Document*> docs;

	QList<GObjectViewState*> objectViews;
    AppResource *resourceTracker;
    QMap<QString, qint64> resourceUsage;

	//Every implementation of Project Parser must be added as friend, to get access to addState
	friend class ProjectParser00;
	friend class ProjectParser10;
};


}//namespace

#endif
