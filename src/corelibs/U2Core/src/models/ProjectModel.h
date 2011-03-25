/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

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

    virtual bool lockResources(int sizeMB, const QString & url, QString &error) = 0;

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
