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

#ifndef _U2_PROJECT_TREE_CONTROLLER_H_
#define _U2_PROJECT_TREE_CONTROLLER_H_

#include <U2Core/global.h>
#include <U2Core/DocumentSelection.h>
#include <U2Core/GObjectSelection.h>

#include <U2Core/AppContext.h>

#include <U2Core/GObjectReference.h>

#include <QtCore/QPointer>
#include <QtGui/QTreeWidget>
#include <QtGui/QIcon>


namespace U2 {

class Task;
class GObjectConstraints;
class MWMDIWindow;
class ProjViewItem;
class ProjViewDocumentItem;	
class ProjViewTypeItem;
class ProjViewObjectItem;
class GObjectView;

class U2GUI_EXPORT LoadDocumentTaskProvider {
public:
    virtual ~LoadDocumentTaskProvider(){};
    virtual QList<Task*> createLoadDocumentTasks(const QList<Document*>& docs) const = 0;
};

enum ProjectTreeGroupMode {
    ProjectTreeGroupMode_Flat,         //objects are not grouped, only unloaded documents are shown
    ProjectTreeGroupMode_ByDocument,   //objects are grouped by document
    ProjectTreeGroupMode_ByType,       //objects are grouped by type
    
    ProjectTreeGroupMode_Min = ProjectTreeGroupMode_Flat,
    ProjectTreeGroupMode_Max = ProjectTreeGroupMode_ByType
};

//filtered objects and documents are not shown in project tree
class PTCObjectFilter : public QObject{
public:
    PTCObjectFilter(QObject* p = NULL) : QObject(p){}
    virtual bool filter(GObject* o) const = 0;
};

class PTCDocumentFilter : public QObject {
public:
    PTCDocumentFilter(QObject* p): QObject(p) {}
    virtual bool filter(Document* d) const = 0;
};


class PTCObjectRelationFilter : public PTCObjectFilter {
public:
    PTCObjectRelationFilter(const GObjectRelation& _rel, QObject* p = NULL) : PTCObjectFilter(p), rel(_rel){}
    bool filter(GObject* o) const;
    GObjectRelation rel;
};



class U2GUI_EXPORT ProjectTreeControllerModeSettings {
public:
    ProjectTreeControllerModeSettings() 
        : allowMultipleSelection(true), readOnlyFilter(TriState_Unknown), loadTaskProvider(NULL), 
        groupMode(ProjectTreeGroupMode_ByDocument), objectFilter(NULL), documentFilter(NULL), 
        markActive(false){}
    
    QList<GObjectType>          objectTypesToShow;  // show only objects of specified type
    QList<GObjectConstraints*>  objectConstraints;  // show only objects that fits constraints
    QList<QPointer<GObject> >   excludeObjectList;  // do not show these objects
    QStringList                 tokensToShow;       // show documents/objects with all of tokens in a name
    bool                        allowMultipleSelection; //use multiple selection in tree
    TriState                    readOnlyFilter;     // unknown->all, true->filter(exclude) readonly, false -> keep only readonly
    LoadDocumentTaskProvider*   loadTaskProvider;   // use custom LoadDocumentTask factory instead of default
    ProjectTreeGroupMode        groupMode;          // group mode for objects

    //Note that objectFilter and documentFilter are called only on object add/remove ops!
    //WARN: object and document filters live-range is controlled by the side created these objects
    PTCObjectFilter*            objectFilter;       
    PTCDocumentFilter*          documentFilter;

    bool                        markActive;
    QFont                       activeFont;
    
    bool isDocumentShown(Document* doc) const;
    bool isTypeShown(GObjectType t) const;
    bool isObjectShown(GObject* o) const ;
};

class U2GUI_EXPORT ProjectTreeController : public QObject {
	Q_OBJECT
public:
	ProjectTreeController(QObject* parent, QTreeWidget* tree, const ProjectTreeControllerModeSettings& mode);
	
	const DocumentSelection* getDocumentSelection() const {return &documentSelection;}
	const GObjectSelection* getGObjectSelection() const {return &objectSelection;}

    void updateSettings(const ProjectTreeControllerModeSettings& mode);

    QAction* getGroupByDocumentAction() const {return groupByDocumentAction;}
    QAction* getGroupByTypeAction() const {return groupByTypeAction;}
    QAction* getGroupFlatAction() const {return groupFlatAction;}
    QAction* getLoadSeletectedDocumentsAction() { return loadSelectedDocumentsAction; }
    const ProjectTreeControllerModeSettings& getModeSettings() const {return mode;}

    void highlightItem(Document*);

protected:
    bool virtual eventFilter(QObject* o, QEvent* e);

signals:
	void si_onPopupMenuRequested(QMenu& popup);
	void si_doubleClicked(GObject*);
    void si_nameChanged(GObject *);

private slots:
	void sl_onTreeSelectionChanged();
	void sl_onContextMenuRequested(const QPoint & pos);
	void sl_onRemoveSelectedDocuments();
	void sl_onLoadSelectedDocuments();
    void sl_onUnloadSelectedDocuments();

	void sl_onDocumentAddedToProject(Document* d);
	void sl_onDocumentRemovedFromProject(Document* d);
	void sl_onDocumentModifiedStateChanged();
	void sl_onDocumentLoadedStateChanged();
    void sl_onDocumentURLorNameChanged();
    
	void sl_onObjectAdded(GObject* o);
	void sl_onObjectRemoved(GObject* o);
	void sl_onObjectModifiedStateChanged();

	void sl_onItemDoubleClicked(QTreeWidgetItem * item, int column);
    void sl_onItemChanged(QTreeWidgetItem *, int );

    void sl_onResourceUserRegistered(const QString& res, Task* t);
    void sl_onResourceUserUnregistered(const QString& res, Task* t);
    void sl_onLoadingDocumentProgressChanged();
    void sl_onToggleReadonly();
    void sl_lockedStateChanged();
    
    void sl_onGroupByDocument();
    void sl_onGroupByType();
    void sl_onGroupFlat();

    void sl_windowActivated(MWMDIWindow*);
    void sl_objectAddedToActiveView(GObjectView*, GObject*);
    void sl_objectRemovedFromActiveView(GObjectView*, GObject*);

private:
	void updateActions();
	void updateSelection();
	void connectModel();
	void connectDocument(Document* d);
	void disconnectDocument(Document* d);
    void connectToResourceTracker();
	void connectGObject(GObject* d);
	void buildTree();
	void buildDocumentTree(Document* d);
    void flattenDocumentItem(ProjViewDocumentItem* docItem);
    void runLoadDocumentTasks(const QList<Document*>& docs);
    GObjectType getLoadedObjectType(GObject* obj) const;
    void filterItemsRecursive(ProjViewItem* pi);
    QSet<Document*>  getDocsInSelection(bool deriveFromObjects);
    void insertTreeItemSorted(ProjViewItem* p, ProjViewItem* item);
    void updateLoadingState(Document* d);
    void updateObjectActiveStateVisual(GObject* o);
    

    ProjViewDocumentItem* findDocumentItem(Document* d) const;
    ProjViewDocumentItem* findDocumentItem(Document* d, bool create);
    ProjViewObjectItem* findGObjectItem(GObject* o) const;
    ProjViewObjectItem* findGObjectItem(Document* d, GObject* o) const;
    ProjViewObjectItem* findGObjectItem(ProjViewItem* pi, GObject* o) const;
    ProjViewTypeItem*   findTypeItem(const GObjectType& t) const;
    ProjViewTypeItem*   findTypeItem(const GObjectType& t, bool create);


	QTreeWidget* tree;

	QAction* removeSelectedDocumentsAction;
	QAction* loadSelectedDocumentsAction;
    QAction* unloadSelectedDocumentsAction;
    QAction* addReadonlyFlagAction;
    QAction* removeReadonlyFlagAction;
    
    QAction* groupByDocumentAction;
    QAction* groupByTypeAction;
    QAction* groupFlatAction;

	GObjectSelection                    objectSelection;
	DocumentSelection                   documentSelection;
    ProjectTreeControllerModeSettings   mode;
    QSet<ProjViewItem*>                 itemsToUpdate;
    bool                                editing;
    GObjectView*                        markActiveView;


public: 
	QIcon documentIcon;
    QIcon roDocumentIcon;
};

class U2GUI_EXPORT ProjViewItem : public QTreeWidgetItem , public QObject{
public:
	ProjViewItem(ProjectTreeController* c) :  QObject(NULL), controller(c), markedAsActive(false) {}
    bool isRootItem() {return QTreeWidgetItem::parent() == NULL;}
	virtual bool isDocumentItem() const {return false;}
    virtual bool isTypeItem() const {return false;}
	virtual bool isObjectItem() const {return false;}

    // checks if item must be marked as active
    virtual bool isActive() const {return false;}
    
    // updates item visual state
	virtual void updateVisual(bool recursive = false) = 0;

    // updates active item visual state only
    virtual void updateActive();

    virtual QTreeWidgetItem *parent() {return QTreeWidgetItem::parent();}
	
    ProjectTreeController*  controller;
    bool                    markedAsActive;
};

class U2GUI_EXPORT ProjViewTypeItem : public ProjViewItem {
public:
    ProjViewTypeItem(const GObjectType& t, ProjectTreeController* c);
    virtual bool isTypeItem() const {return true;}
    virtual bool operator< ( const QTreeWidgetItem & other ) const;
    virtual void updateVisual(bool recursive = false);
    GObjectType otype;
    QString     typePName;
};


class U2GUI_EXPORT ProjViewDocumentItem : public ProjViewItem {
public:
	ProjViewDocumentItem(Document* _doc, ProjectTreeController* c);
	virtual bool isDocumentItem() const {return true;}
	virtual void updateVisual(bool recursive = false);
    bool isActive() const;
    virtual bool operator< ( const QTreeWidgetItem & other ) const;
    Document* doc;
};

class U2GUI_EXPORT ProjViewObjectItem : public ProjViewItem {
public: 
	ProjViewObjectItem(GObject* _obj, ProjectTreeController* c);
	virtual bool isObjectItem() const {return true;}
	virtual void updateVisual(bool recursive = false);
    bool isActive() const;
    virtual bool operator< ( const QTreeWidgetItem & other ) const;
    virtual void setData(int column, int role, const QVariant& value);
    GObject* obj;
	
};


}//namespace
#endif
