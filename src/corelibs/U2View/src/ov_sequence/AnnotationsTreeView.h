/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_ANNOTATIONS_TREE_VIEW_
#define _U2_ANNOTATIONS_TREE_VIEW_

#include <U2Core/global.h>
#include <U2Core/Task.h>

#include <U2View/SearchQualifierDialog.h>

#include <QtCore/QFlags>
#include <QtCore/QTimer>
#include <QtGui/QTreeWidget>
#include <QtGui/QCloseEvent>
#include <QtGui/QLabel>
#include <QtCore/QQueue>
#include <QtCore/QPair>


namespace U2 {

class Annotation;
class AnnotationGroup;
class U2Qualifier;
class AnnotationTableObject;
class AVItem;
class AVGroupItem;
class AVAnnotationItem;
class AVQualifierItem;
class AnnotatedDNAView;
class AnnotationSelection;
class AnnotationGroupSelection;
class AnnotationModification;
class GObjectView;
class RemoveItemsTask;

enum ATVAnnUpdateFlag {
    ATVAnnUpdateFlag_BaseColumns = 0x1,
    ATVAnnUpdateFlag_QualColumns = 0x2
};

typedef QFlags<ATVAnnUpdateFlag> ATVAnnUpdateFlags;


class U2VIEW_EXPORT AnnotationsTreeView : public QWidget {
    Q_OBJECT
public:
    AnnotationsTreeView(AnnotatedDNAView* ctx);

    void saveWidgetState();
    void restoreWidgetState();

    void adjustStaticMenu(QMenu *m) const {adjustMenu(m);}

    QTreeWidget* getTreeWidget() const {return tree;}
    
    QStringList getQualifierColumnNames() const {return qColumns;}
    void addQualifierColumn(const QString& q);
    void removeQualifierColumn(const QString& q);

    void saveState(QVariantMap& map) const;
    void updateState(const QVariantMap& map);

    void setSortingEnabled(bool v);

    AVItem* currentItem();

private slots:

    void sl_onAnnotationObjectAdded(AnnotationTableObject* obj);
    void sl_onAnnotationObjectRemoved(AnnotationTableObject* obj);
    void sl_onAnnotationObjectRenamed(const QString &oldName);

    void sl_onAnnotationsAdded(const QList<Annotation*>&);
    void sl_onAnnotationsRemoved(const QList<Annotation*>&);
    void sl_onAnnotationsInGroupRemoved(const QList<Annotation*>&, AnnotationGroup*);
    void sl_onAnnotationModified(const AnnotationModification& md);
    void sl_annotationObjectModifiedStateChanged();

    void sl_onGroupCreated(AnnotationGroup*);
    void sl_onGroupRemoved(AnnotationGroup* parent, AnnotationGroup* removed);
    void sl_onGroupRenamed(AnnotationGroup*, const QString& oldName);

    void sl_onAnnotationSettingsChanged(const QStringList& changedSettings);

    void sl_onAnnotationSelectionChanged(AnnotationSelection*, const QList<Annotation*>&, const QList<Annotation*>&);
    void sl_onAnnotationGroupSelectionChanged(AnnotationGroupSelection*, const QList<AnnotationGroup*>&, const QList<AnnotationGroup*>&);
    void sl_onItemSelectionChanged();
    void sl_onAddAnnotationObjectToView();
    void sl_removeObjectFromView();
    void sl_removeAnnsAndQs();
    void sl_onBuildPopupMenu(GObjectView* thiz, QMenu* menu);
    void sl_onCopyQualifierValue();
    void sl_onCopyQualifierURL();
    void sl_onToggleQualifierColumn();
    void sl_onRemoveColumnByHeaderClick();
    void sl_onCopyColumnText();
    void sl_onCopyColumnURL();
	void sl_exportAutoAnnotationsGroup();
    void sl_searchQualifier();
    
    void sl_rename();
    void sl_edit();
    void sl_addQualifier();
//    void sl_cutAnnotations();
//    void sl_copyAnnotations();
//    void sl_pasteAnnotations();

    void sl_itemEntered(QTreeWidgetItem * i, int column);
    void sl_itemClicked( QTreeWidgetItem * item, int column);
    void sl_itemDoubleClicked (QTreeWidgetItem * item, int column);
    void sl_itemExpanded(QTreeWidgetItem*);
    
    //TODO: deal with style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick) correctly
    //    void sl_itemActivated(QTreeWidgetItem*, int) {sl_edit();}
    void sl_sortTree();

protected:
    bool eventFilter(QObject* o, QEvent* e);

private:
    void renameItem(AVItem* i);
    void editItem(AVItem* i);
    
    QString renameDialogHelper(AVItem* i, const QString& defText, const QString& title);
    bool editQualifierDialogHelper(AVQualifierItem* i, bool ro, U2Qualifier& res);
    void moveDialogToItem(QTreeWidgetItem* item, QDialog& d);
    
    void adjustMenu(QMenu* m_) const;
    AVGroupItem* buildGroupTree(AVGroupItem* parentGroup, AnnotationGroup* g);
    AVAnnotationItem* buildAnnotationTree(AVGroupItem* parentGroup, Annotation* a);
    void populateAnnotationQualifiers(AVAnnotationItem* ai);
    void updateAllAnnotations(ATVAnnUpdateFlags flags);
	QMenu* getAutoAnnotationsHighligtingMenu(AnnotationTableObject* aObj);

    AVGroupItem* findGroupItem(const AnnotationGroup* g) const;
    AVAnnotationItem* findAnnotationItem(const AnnotationGroup* g, const Annotation* a) const;
    AVAnnotationItem* findAnnotationItem(const AVGroupItem* gi, const Annotation* a) const;
    // searches for annotation items that has not-null document added to the view
    QList<AVAnnotationItem*> findAnnotationItems(const Annotation* a) const;
    
    void connectAnnotationSelection();
    void connectAnnotationGroupSelection();
    
    void updateState();
    void updateColumnContextActions(AVItem* item, int col);

    void resetDragAndDropData();
    bool initiateDragAndDrop(QMouseEvent* me);
    void finishDragAndDrop(Qt::DropAction dndAction);

    QTreeWidget* tree;

    AnnotatedDNAView*   ctx;
    QAction*            addAnnotationObjectAction;
    QAction*            removeObjectsFromViewAction;
    QAction*            removeAnnsAndQsAction;
    QAction*            copyQualifierAction;
    QAction*            copyQualifierURLAction;
    QAction*            toggleQualifierColumnAction;
    QAction*            removeColumnByHeaderClickAction;
    QAction*            copyColumnTextAction;
    QAction*            copyColumnURLAction;
	QAction*			exportAutoAnnotationsGroup;
//    QAction*            cutAnnotationsAction;
//    QAction*            copyAnnotationsAction;
//    QAction*            pasteAnnotationsAction;
    
    QAction*            renameAction;       // action to rename active group/qualifier/annotation only
    QAction*            editAction;         // action to edit active item -> only for non-readonly
    QAction*            viewAction;         // action to view active item -> could be used both for readonly and not readonly
    QAction*            addQualifierAction; // action to create qualifier. Editable annotation or editable qualifier must be selected

    QAction*            searchQualifierAction; 
    
    Qt::MouseButton     lastMB;
    QStringList         headerLabels;
    QStringList         qColumns;
    int                 lastClickedColumn;
    QIcon               addColumnIcon;
    QIcon               removeColumnIcon;
    QTimer              sortTimer;
    QPoint              dragStartPos;
	QMenu*				highlightAutoAnnotationsMenu;
    // drag&drop related data
    bool                    isDragging;
    bool                    dndCopyOnly;
    QList<AVItem*>          dndSelItems;
    AVGroupItem*            dropDestination;

    static const QString annotationMimeType;

    friend class RemoveItemsTask;
    friend class FindQualifierTask;
    friend class SearchQualifierDialog;
};

//////////////////////////////////////////////////////////////////////////
/// Tree model

//TODO: create qualifiers subtrees only when qualifier node is opened (usually qualifiers get ~ 90% of memory)

enum AVItemType {
    AVItemType_Group,
    AVItemType_Annotation,
    AVItemType_Qualifier
};

class AVItem : public QTreeWidgetItem {
public:
    AVItem(QTreeWidgetItem* parent, AVItemType _type) : QTreeWidgetItem(parent), type(_type) {}
    const AVItemType type;
    bool processLinks(const QString& qname, const QString& qval, int col);
    bool isColumnLinked(int col) const;
    QString buildLinkURL(int col) const;
    QString getFileUrl(int col) const;
    virtual bool isReadonly() const  {assert(parent()!=NULL); return (static_cast<AVItem*>(parent())->isReadonly());}

    virtual AnnotationsTreeView* getAnnotationTreeView() const {assert(parent()!=NULL); return (static_cast<AVItem*>(parent())->getAnnotationTreeView());}
    virtual AnnotationTableObject* getAnnotationTableObject() const {assert(parent()!=NULL); return (static_cast<AVItem*>(parent())->getAnnotationTableObject());}
    virtual AnnotationGroup* getAnnotationGroup() const {assert(parent()!=NULL); return (static_cast<AVItem*>(parent())->getAnnotationGroup());}

};


class AVGroupItem : public AVItem {
public:
    AVGroupItem(AnnotationsTreeView* atv, AVGroupItem* parent, AnnotationGroup* g);
    ~AVGroupItem();

    void updateVisual();
    void updateAnnotations(const QString& nameFilter, ATVAnnUpdateFlags flags);
    void findAnnotationItems(QList<AVAnnotationItem*>& result, Annotation* a) const;

    static const QIcon& getGroupIcon();
    static const QIcon& getDocumentIcon();
    virtual AnnotationsTreeView* getAnnotationTreeView() const  {return atv;}
    virtual bool isReadonly() const;
    virtual AnnotationTableObject* getAnnotationTableObject() const;
    virtual AnnotationGroup* getAnnotationGroup() const;

    AnnotationGroup* group;
    AnnotationsTreeView* atv;
};

class AVAnnotationItem : public AVItem {
public:
    AVAnnotationItem(AVGroupItem* parent, Annotation* a);
    ~AVAnnotationItem();
    Annotation* annotation;
    mutable QString locationString;

    virtual QVariant data ( int column, int role ) const;
    void updateVisual(ATVAnnUpdateFlags flags);
    virtual bool operator< ( const QTreeWidgetItem & other ) const;
    bool isColumnNumeric(int col) const;
    double getNumericVal(int col) const;
    
    void removeQualifier(const U2Qualifier& q);
    void addQualifier(const U2Qualifier& q);
    AVQualifierItem* findQualifierItem(const QString& name, const QString& val) const;
    
    static QMap<QString, QIcon>& getIconsCache();
    bool hasNumericQColumns;
};

class AVQualifierItem: public AVItem {
public:
    AVQualifierItem(AVAnnotationItem* parent, const U2Qualifier& q);

    //TODO: keep values in U2Qualifier struct
    const QString qName;
    const QString qValue;
};

class RemoveItemsTask: public Task  {
public:
    RemoveItemsTask(AnnotationsTreeView *_treeView, AnnotationTableObject *_aObj, const QList<Annotation *>& list, AnnotationGroup *gr):
    Task("Remove items", TaskFlag_None), treeView(_treeView), aObj(_aObj), as(list), parentGroup(gr), parentGroupItem(NULL) {}
    void prepare();
    void run();
    Task::ReportResult report();

private:
    AnnotationsTreeView *treeView;
    AnnotationTableObject* aObj;
    QList<Annotation *> as;
    AnnotationGroup *parentGroup;

    QSet<AVGroupItem*> groupsToUpdate;
    AVGroupItem* parentGroupItem;
    Qt::ItemFlags flags;
    QList<AVAnnotationItem *> itemsToDelete;
};

class FindQualifierTaskSettings{
public:
    FindQualifierTaskSettings(AVItem* _groupToSearchIn, const QString& _name, const QString& _value, bool _isExactMatch, bool _searchAll, AVItem* _prevAnnotation = NULL, int _prevIndex = -1)
    :groupToSearchIn(_groupToSearchIn)
    ,name(_name)
    ,value(_value)
    ,isExactMatch(_isExactMatch)
    ,prevAnnotation(_prevAnnotation)
    ,prevIndex(_prevIndex)
    ,searchAll(_searchAll)
    {
    }

    AVItem* groupToSearchIn;
    QString name;
    QString value;
    bool isExactMatch;
    AVItem* prevAnnotation;
    int prevIndex;
    bool searchAll;
};

class U2VIEW_EXPORT FindQualifierTask: public Task{
    Q_OBJECT
public:
    FindQualifierTask(AnnotationsTreeView * _treeView, const FindQualifierTaskSettings& settings);
    void prepare();
    void run();
    ReportResult report();

    int getIndexOfResult() const { return indexOfResult; }
    AVItem * getResultAnnotation() const { return resultAnnotation; }
    bool isFound() const {return foundResult; }

private:
    void findInAnnotation(AVItem* annotation, bool& found);
    void findInGroup(AVItem* group, bool& found);
    int getStartIndexGroup(AVItem* group);
    int getStartIndexAnnotation(AVItem* annotation);

    AnnotationsTreeView *       treeView;
    QString                     qname;
    QString                     qvalue;
    AVItem *                    groupToSearchIn;
    bool                        isExactMatch;
    bool                        searchAll;
    bool                        foundResult;

    int indexOfResult;
    AVItem * resultAnnotation;

    QQueue<AVItem*> toExpand; //this queue is needed to expand items in main thread
    QList< QPair<AVAnnotationItem*, int> > foundQuals; //this is needed to set found items as selected
};

}//namespace


#endif
