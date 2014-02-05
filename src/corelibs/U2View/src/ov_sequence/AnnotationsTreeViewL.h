/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <QtCore/QFlags>
#include <QtCore/QTimer>
#include <QtGui/QTreeWidget>
#include <QtGui/QCloseEvent>
#include <QtGui/QLabel>
#include <QtGui/QScrollBar>
#include <QtGui/QAbstractSlider>
#include <QtGui/QStyle>
#include <QtGui/QPainter>
#include <QtGui/QApplication>

#include "AnnotationsTreeModel.h"

namespace U2 {

class U2Qualifier;
class FeaturesTableObject;
class AVItemL;
class AVGroupItemL;
class AVAnnotationItemL;
class AVQualifierItemL;
class AnnotatedDNAView;
class AnnotationSelection;
class AnnotationGroupSelection;
class AnnotationModification;
class GObjectView;
class LazyTreeView;
class AnnotationsTreeViewL;


class CustomSlider: public QScrollBar {
public:
    CustomSlider(QWidget *parent = NULL);
    void setMaxVal(int _maxVal);
    int getMaxVal() const;
    void setPosition(int pos);
    void setCustomPageStep(int ps);
    int sliderCustomPosition() const;
    void setRowHeight(int height);
    int numToScroll() const;
    void resetNumToScroll();

protected:
    virtual void sliderChange(SliderChange sc);
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *me);
    void mousePressEvent(QMouseEvent *me);
    void mouseReleaseEvent(QMouseEvent *me);

private:
    int minVal;
    int maxVal;
    int sliderPos;
    //int pageStep;
    int rowHeight;
    QStyleOptionSlider options;
    int dif;
    bool sliderPressed;
};


class U2VIEW_EXPORT LazyTreeView: public QTreeView {
    Q_OBJECT
public:
    LazyTreeView(QWidget * parent = 0 );

    QList<QTreeWidgetItem*> selectedItems() ;
    QTreeWidgetItem* currentItem() ;
    void scrollToItem(QTreeWidgetItem *item, ScrollHint hint = EnsureVisible) ;
    void setCurrentItem(QTreeWidgetItem *item) ;
    int topLevelItemCount() const ;
    QTreeWidgetItem * topLevelItem(int i) ;
    void setColumnCount(int nCol);
    void setHeaderLabels(QStringList headers);
    QTreeWidgetItem *itemAt(const QPoint pos);
    int getMaxItem() const {return numOnScreen;}
    
    int getPositionInTree(QTreeWidgetItem *item);

    AVItemL *getNextItemDown(AVItemL * bottom);
    AVItemL *getNextItemUp();
    AVItemL *getLastItemInSubtree( const __AnnotationGroup &gr, AnnotationsTreeViewL *view );
    int getExpandedNumber(AVItemL *item);
    int getExpandedNumber( const __AnnotationGroup &gr );
    void updateItem(QTreeWidgetItem *item);
    void setLineHeight(int height);

    QModelIndex guessIndex(QTreeWidgetItem *item);

protected:
    virtual void selectionChanged ( const QItemSelection & selected, const QItemSelection & deselected );
    virtual void mouseDoubleClickEvent ( QMouseEvent * event );
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void scrollContentsBy ( int dx, int dy );
    int scrollOneItemDown();
    int scrollOneItemUp();

private:
    void resizeModel();
    void insertItem(int row, QTreeWidgetItem *item, bool removeLast = true);
    void insertItemBehindView(int row, QTreeWidgetItem *item);
    void removeItem(QTreeWidgetItem *item, bool removeAll = false);
    void calculateIndex(AnnotationTableObject *obj);
    void updateSlider();
    
private slots:
    void sl_expanded(const QModelIndex &index) ;
    void sl_collapsed(const QModelIndex &index);
    void sl_entered(const QModelIndex &index);

signals:
    void itemSelectionChanged();
    void itemDoubleClicked(QTreeWidgetItem*, int);
    void itemClicked(QTreeWidgetItem*, int);
    void itemExpanded(QTreeWidgetItem*);
    void itemCollapsed(QTreeWidgetItem*);
    void itemEntered(QTreeWidgetItem*, int);

private:
    QList<QTreeWidgetItem*> onScreen;
    int numOnScreen;
    int realNumberOfItems;
    bool flag;
    bool emptyExpand;
    int lineHeight;
    TreeIndex *treeWalker;

    friend class AnnotationsTreeViewL;
};

class U2VIEW_EXPORT AnnotationsTreeViewL : public QWidget {
    Q_OBJECT
public:
    AnnotationsTreeViewL(AnnotatedDNAView* ctx);

    void saveWidgetState();
    void restoreWidgetState();

    void adjustStaticMenu(QMenu *m) const {adjustMenu(m);}

    QTreeView* getTreeWidget() const {return tree;}
    
    QStringList getQualifierColumnNames() const {return qColumns;}
    void addQualifierColumn(const QString& q);
    void removeQualifierColumn(const QString& q);

    void saveState(QVariantMap& map) const;
    void updateState(const QVariantMap& map);

    void setSortingEnabled(bool v);

    AVItemL* currentItem();

private slots:

    void sl_onAnnotationObjectAdded( FeaturesTableObject *obj );
    void sl_onAnnotationObjectRemoved( FeaturesTableObject *obj );

    void sl_onAnnotationsAdded( const QList<__Annotation> & );
    void sl_onAnnotationsRemoved( const QList<__Annotation> & );
    void sl_onAnnotationsInGroupRemoved( const QList<__Annotation>& as, const __AnnotationGroup &group );
    void sl_onAnnotationModified( const AnnotationModification &md );
    void sl_onAnnotationObjectModifiedStateChanged( );
    void sl_onAnnotationObjectRenamed( const QString &oldName );


    void sl_onGroupCreated( const __AnnotationGroup & );
    void sl_onGroupRemoved( const __AnnotationGroup &parent, const __AnnotationGroup &removed);
    void sl_onGroupRenamed( const __AnnotationGroup & );

    void sl_onAnnotationSelectionChanged( AnnotationSelection*, const QList<__Annotation> &,
        const QList<__Annotation> & );
    void sl_onAnnotationGroupSelectionChanged( AnnotationGroupSelection *, const QList<__AnnotationGroup> &,
        const QList<__AnnotationGroup> & );
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
    
    void sl_rename();
    void sl_edit();
    void sl_addQualifier();

    void sl_itemEntered(QTreeWidgetItem * i, int column);
    void sl_itemClicked( QTreeWidgetItem * item, int column);
    void sl_itemDoubleClicked (QTreeWidgetItem * item, int column);
    void sl_itemExpanded(QTreeWidgetItem*);
    void sl_itemCollapsed(QTreeWidgetItem*);

protected:
    bool eventFilter(QObject* o, QEvent* e);

private:
    void renameItem(AVItemL* i);
    void editItem(AVItemL* i);
    
    QString renameDialogHelper(AVItemL* i, const QString& defText, const QString& title);
    bool editQualifierDialogHelper(AVQualifierItemL* i, bool ro, U2Qualifier& res);
    void moveDialogToItem(QTreeWidgetItem* item, QDialog& d);
    
    void adjustMenu(QMenu* m_) const;
    AVGroupItemL* createGroupItem(AVGroupItemL* parentGroup, const __AnnotationGroup &g);
    AVAnnotationItemL* createAnnotationItem(AVGroupItemL* parentGroup, const __Annotation &a,
        bool removeLast = true);
    void updateAllAnnotations(ATVAnnUpdateFlags flags);
    void destroyTree(QTreeWidgetItem* qi);
    void focusOnItem(Annotation* a);

    AVGroupItemL* findGroupItem(const AnnotationGroup* g) const;
    AVAnnotationItemL* findAnnotationItem(const AnnotationGroup* g, const Annotation* a) const;
    AVAnnotationItemL* findAnnotationItem(const AVGroupItemL* gi, const Annotation* a) const;
    // searches for annotation items that has not-null document added to the view
    QList<AVAnnotationItemL*> findAnnotationItems(const Annotation* a) const;
    
    void connectAnnotationSelection();
    void connectAnnotationGroupSelection();
    
    void updateState();
    void updateColumnContextActions(AVItemL* item, int col);

    void resetDragAndDropData();
    bool initiateDragAndDrop(QMouseEvent* me);
    void finishDragAndDrop(Qt::DropAction dndAction);

    LazyTreeView* tree;

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

    QAction*            renameAction;       // action to rename active group/qualifier/annotation only
    QAction*            editAction;         // action to edit active item -> only for non-readonly
    QAction*            viewAction;         // action to view active item -> could be used both for readonly and not readonly
    QAction*            addQualifierAction; // action to create qualifier. Editable annotation or editable qualifier must be selected
    
    Qt::MouseButton     lastMB;
    QStringList         headerLabels;
    QStringList         qColumns;
    int                 lastClickedColumn;
    QIcon               addColumnIcon;
    QIcon               removeColumnIcon;
    QTimer              sortTimer;
    QPoint              dragStartPos;

    // drag&drop related data
    bool                    isDragging;
    bool                    dndCopyOnly;
    bool                    renameFlag;
    QList<AVItemL*>          dndSelItems;
    AVGroupItemL*            dropDestination;

    static const QString annotationMimeType;
    friend class LazyTreeView;
};


}//namespace


#endif
