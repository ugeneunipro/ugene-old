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

    virtual RawDataCheckResult checkRawData(const QByteArray& rawData, const GUrl& url = GUrl()) const;

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
