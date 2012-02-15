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

#ifndef _ANNOTATIONS_TREE_MODEL_H_
#define _ANNOTATIONS_TREE_MODEL_H_

//#include "AnnotationsTreeViewL.h"

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AnnotationSettings.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationSelection.h>
#include <QtGui/QTreeWidgetItem>
#include <QtCore/QString>
#include <QtCore/QFile>

#include <vector>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
/// Tree model

enum AVItemTypeL {
    AVItemType_Group,
    AVItemType_Annotation,
    AVItemType_Qualifier
};

enum ATVAnnUpdateFlag {
    ATVAnnUpdateFlag_BaseColumns = 0x1,
    ATVAnnUpdateFlag_QualColumns = 0x2
};
typedef QFlags<ATVAnnUpdateFlag> ATVAnnUpdateFlags;


class AnnotationsTreeViewL;
class AVAnnotationItemL;
class AVQualifierItemL;

class AVItemL : public QTreeWidgetItem {
public:
    AVItemL(QTreeWidgetItem* parent, AVItemTypeL _type) : QTreeWidgetItem(parent), type(_type) { }
    virtual ~AVItemL() {}
    const AVItemTypeL type;
    bool processLinks(const QString& qname, const QString& qval, int col);
    bool isColumnLinked(int col) const;
    QString buildLinkURL(int col) const;
    QString getFileUrl(int col) const;
    virtual bool isReadonly() const  {assert(parent()!=NULL); return (static_cast<AVItemL*>(parent())->isReadonly());}
    virtual int childNumber() const {return childCount();}
    bool removeChildren(int position, int count);
    bool addChildren(int position, int count, AVItemL *item);

    virtual AnnotationsTreeViewL* getAnnotationTreeView() const {assert(parent()!=NULL); return (static_cast<AVItemL*>(parent())->getAnnotationTreeView());}
    virtual AnnotationTableObject* getAnnotationTableObject() const {assert(parent()!=NULL); return (static_cast<AVItemL*>(parent())->getAnnotationTableObject());}
    virtual AnnotationGroup* getAnnotationGroup() const {assert(parent()!=NULL); return (static_cast<AVItemL*>(parent())->getAnnotationGroup());}
};

class AVGroupItemL : public AVItemL {
public:
    AVGroupItemL(AnnotationsTreeViewL* atv, AVGroupItemL* parent, AnnotationGroup* g);
    ~AVGroupItemL();

    void updateVisual();
    void updateAnnotations(const QString& nameFilter, ATVAnnUpdateFlags flags);
    void findAnnotationItems(QList<AVAnnotationItemL*>& result, Annotation* a) const;

    static const QIcon& getGroupIcon();
    static const QIcon& getDocumentIcon();
    virtual AnnotationsTreeViewL* getAnnotationTreeView() const  {return atv;}
    virtual bool isReadonly() const;
    virtual AnnotationTableObject* getAnnotationTableObject() const;
    virtual AnnotationGroup* getAnnotationGroup() const;
    virtual int childNumber() const;
    //void changeIndexes(int i);

    AnnotationGroup* group;
    AnnotationsTreeViewL* atv;
};

class AVAnnotationItemL : public AVItemL {
public:
    AVAnnotationItemL(AVGroupItemL* parent, Annotation* a);
    ~AVAnnotationItemL();
    Annotation* annotation;
    mutable QString locationString;

    virtual QVariant data ( int column, int role ) const;
    void updateVisual(ATVAnnUpdateFlags flags);
    virtual bool operator< ( const QTreeWidgetItem & other ) const;
    bool isColumnNumeric(int col) const;
    double getNumericVal(int col) const;
    virtual int childNumber() const;

    void removeQualifier(const U2Qualifier& q);
    void addQualifier(const U2Qualifier& q);
    AVQualifierItemL* findQualifierItem(const QString& name, const QString& val) const;

    static QMap<QString, QIcon>& getIconsCache();
    bool hasNumericQColumns;
};

class AVQualifierItemL: public AVItemL {
public:
    virtual int childNumber() const ;
    AVQualifierItemL(AVAnnotationItemL* parent, const U2Qualifier& q);

    const QString qName;
    const QString qValue;
};

class TreeIndex {
public:
    TreeIndex();
    ~TreeIndex();
    bool isExpanded(AVItemL *item);
    bool isExpanded(Annotation *a, AnnotationGroup *gr);
    bool isExpanded(AnnotationGroup *gr);
    void expand(QTreeWidgetItem *i);
    void collapse(QTreeWidgetItem *i);
    int getChildNumber(AnnotationTableObject * obj);
    void recalculate( AnnotationTableObject * obj );
    int getChildNumber(AnnotationGroup *gr, int index);
    int getChildNumber(AnnotationGroup *gr);
    int findPosition(AnnotationGroup *gr);
    void deleteItem(Annotation *a, AnnotationGroup *gr);
    void deleteItem(AnnotationGroup *gr);
    void addItem(Annotation *a, AnnotationGroup *gr);
    void addItem(AnnotationGroup *gr);
    QString getRootGroupName(AnnotationTableObject *aobj) const;
    //bool *items;
    //std::vector<char> items;
    //short *indexes;
    //int num;
    QMap<QString, std::vector<char> > itemMap;
};

class U2VIEW_EXPORT LazyAnnotationTreeViewModel: public QAbstractItemModel {
public:
    LazyAnnotationTreeViewModel(QObject * parent = 0);
    virtual ~LazyAnnotationTreeViewModel() {
        delete rootItem;
    }
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual int	columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int	rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QModelIndex	parent ( const QModelIndex & index ) const;
    virtual bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    virtual bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex() );
    virtual bool insertColumns(int position, int columns, const QModelIndex &parent = QModelIndex());
    virtual bool removeColumns(int position, int columns, const QModelIndex &parent = QModelIndex());
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
    virtual bool hasChildren ( const QModelIndex & parent = QModelIndex() ) const;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    AVItemL *getItem(const QModelIndex &index) const;
    QModelIndex guessIndex(QTreeWidgetItem *item);
    int getCurrentItemInModel() const {return currentInModel;}

    void setColumnCount(int nCol);
    void setHeaderLabels(QStringList headers);

    //void insertToplevelItem (AVItemL *item);
    AVItemL *nextToAdd;
    bool debugFlag;
private:
    int nCol;
    QStringList headers;

    int currentInModel;
    int top, bottom;
    int scroll;
    AVItemL *rootItem;
    QModelIndex topIndex;
    QModelIndex bottomIndex;
};


}


#endif
