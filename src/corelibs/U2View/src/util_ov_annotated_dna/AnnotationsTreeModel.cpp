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

#include "AnnotationsTreeModel.h"
#include "AnnotationsTreeViewL.h"
#include <U2Core/DBXRefRegistry.h>
#include <U2Gui/GUIUtils.h>
#include <U2Formats/GenbankLocationParser.h>

namespace U2 {

LazyAnnotationTreeViewModel::LazyAnnotationTreeViewModel(QObject * parent): QAbstractItemModel(parent) {
    rootItem = new AVItemL(NULL, AVItemType_Group);
    nCol = 1;
    headers = QStringList() << "Name" << "Value";
    currentInModel = 0;
    debugFlag = false;
}

AVItemL * LazyAnnotationTreeViewModel::getItem(const QModelIndex &index) const {
    AVItemL *item = rootItem;
    if (index.isValid()) {
        item = static_cast<AVItemL *>(index.internalPointer());
        assert(item);
    }
    return item;
}

QVariant LazyAnnotationTreeViewModel::data ( const QModelIndex & index, int role) const {
    AVItemL *item = getItem(index);
    return item->data(index.column(), role);    
}

int	LazyAnnotationTreeViewModel::columnCount ( const QModelIndex &) const {
    return nCol;

}

int	LazyAnnotationTreeViewModel::rowCount ( const QModelIndex & parent) const {
    AVItemL *parentItem = getItem(parent);
    return parentItem->childCount();

}

QModelIndex	LazyAnnotationTreeViewModel::parent ( const QModelIndex & index ) const {
    if (!index.isValid())
        return QModelIndex();

    AVItemL *childItem = getItem(index);
    AVItemL *parentItem = static_cast<AVItemL *>(childItem->parent());

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

Qt::ItemFlags LazyAnnotationTreeViewModel::flags ( const QModelIndex &) const {
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex LazyAnnotationTreeViewModel::index ( int row, int column, const QModelIndex & parent) const {
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();
    AVItemL *parentItem = getItem(parent);

    AVItemL *childItem = static_cast<AVItemL *>(parentItem->child(row));
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    else {
        return QModelIndex();
    }
}

QVariant LazyAnnotationTreeViewModel::headerData ( int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        return headers[section];
    } else {
        return QVariant();
    }
}

bool LazyAnnotationTreeViewModel::removeRows(int position, int rows, const QModelIndex &parent){
    assert(debugFlag);
    debugFlag = false;
    AVItemL *parentItem = getItem(parent);
    bool success = true;

    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();

    currentInModel -= rows;

    return success;
}

bool LazyAnnotationTreeViewModel::insertRows(int position, int rows, const QModelIndex &parent){
    assert(debugFlag);
    debugFlag = false;
    assert(nextToAdd);
    AVItemL *parentItem = getItem(parent);
    bool success = true;

    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->addChildren(position, rows, nextToAdd);
    endInsertRows();
    nextToAdd = NULL;
    currentInModel += rows;

    return success;
}

QModelIndex LazyAnnotationTreeViewModel::guessIndex(QTreeWidgetItem* item) {
    AVItemL * av = static_cast<AVItemL*>(item);
    if(av->parent()) {
        int row = av->parent()->indexOfChild(av);
        int column = 0;
        QModelIndex index = createIndex(row, column, item);
        return index; 
    } else {
        return QModelIndex();
    }
}

void LazyAnnotationTreeViewModel::setColumnCount(int _nCol) {
    nCol = _nCol;
}

void LazyAnnotationTreeViewModel::setHeaderLabels(QStringList _headers) {
    headers = _headers;
}

bool LazyAnnotationTreeViewModel::hasChildren ( const QModelIndex & parent) const {
    AVItemL *item = getItem(parent);
    if(item->type == AVItemType_Group && item->parent()) {
        AVGroupItemL *ann = static_cast<AVGroupItemL *>(item);
        return (!ann->getAnnotationGroup()->getSubgroups().isEmpty() || !ann->getAnnotationGroup()->getAnnotations().isEmpty());
    }
    if(item->type == AVItemType_Annotation) {
        AVAnnotationItemL *ann = static_cast<AVAnnotationItemL *>(item);
        return (!ann->annotation->getQualifiers().isEmpty());
    } else {
        return (item->childCount() > 0);
    }
}

//Tree Index

TreeIndex::TreeIndex() {
}

TreeIndex::~TreeIndex() {
}

bool TreeIndex::isExpanded(AVItemL *item) {
    switch(item->type) {
        case AVItemType_Group: 
            {
                AVGroupItemL *gr = static_cast<AVGroupItemL*>(item);
                return isExpanded(gr->getAnnotationGroup());
            }
            break;
        case AVItemType_Annotation:
            {
                AVGroupItemL *gr = static_cast<AVGroupItemL*>(item->parent());
                AVAnnotationItemL * ann = static_cast<AVAnnotationItemL*>(item);
                return isExpanded(ann->annotation, gr->getAnnotationGroup());
            }
            break;
        default:
            return 0;
    }
}

bool TreeIndex::isExpanded(Annotation *a, AnnotationGroup *gr) {
    int ind = findPosition(gr) + gr->getAnnotations().indexOf(a) + 1; 
    return (itemMap[getRootGroupName(gr->getGObject())][ind] == 1);
}

bool TreeIndex::isExpanded(AnnotationGroup *gr) {
    if(gr->getParentGroup() == NULL) {
        return (itemMap[getRootGroupName(gr->getGObject())][0] == 1);
    }
    int ind = findPosition(gr);
    return (itemMap[getRootGroupName(gr->getGObject())][ind] == 1);
}


void TreeIndex::expand(QTreeWidgetItem *item) {
    QList<int> indexes;
    QList<AnnotationGroup*> groups;
    AVItemL *av = static_cast<AVItemL*>(item);
    QString objName;
    if(av->type == AVItemType_Group) {
        AnnotationGroup *topGroup = (static_cast<AVGroupItemL*>(av))->getAnnotationGroup();
        int ind = findPosition(topGroup);
        //itemMap[topGroup->getGObject()->getDocument()->getURLString()][ind] = 1;
        itemMap[getRootGroupName(topGroup->getGObject())][ind] = 1;
    } else if(av->type == AVItemType_Annotation) {
        Annotation *a = (static_cast<AVAnnotationItemL*>(av))->annotation;
        AnnotationGroup *topGroup = (static_cast<AVGroupItemL*>(av->parent()))->getAnnotationGroup();
        int offset = topGroup->getAnnotations().indexOf(a);
        int ind = findPosition(topGroup) + offset + 1;
        itemMap[getRootGroupName(topGroup->getGObject())][ind] = 1;
    }
}

int TreeIndex::getChildNumber(AnnotationGroup *gr, int index) {
    int res = 1;
    for(int i = 0; i < index; i++) {
        res += getChildNumber(gr->getSubgroups()[i]);
    }
    res += gr->getAnnotations().size();
    return res;
}

int TreeIndex::getChildNumber(AnnotationGroup *gr) {
    int res = 1;
    for(int i = 0; i < gr->getSubgroups().size(); i++) {
        res += getChildNumber(gr->getSubgroups()[i]);
    }
    res += gr->getAnnotations().size();
    return res;
}

int TreeIndex::findPosition(AnnotationGroup *gr) {
    QList<int> indexes;
    QList<AnnotationGroup*> groups;
    int res = 0;

    /*------------------------------------------*/
    /*-----Array of indexes in tree------------*/
    /*------------------------------------------*/
    AnnotationGroup *parGroup = gr->getParentGroup();
    groups.append(parGroup);
    if(parGroup != NULL) {
        indexes << parGroup->getSubgroups().indexOf(gr);
        while(parGroup->getParentGroup()) {
            indexes.prepend(parGroup->getParentGroup()->getSubgroups().indexOf(parGroup));
            parGroup = parGroup->getParentGroup();
            groups.prepend(parGroup);            
        }
    } else {
        return 0;
    }

    for(int i = 0; i < groups.size(); i++) {
        res += getChildNumber(groups[i], indexes[i]);
        //if(i > 0) {
            parGroup = groups[i];
        //}
    }
    return res;
}

void TreeIndex::collapse(QTreeWidgetItem *item) {
    QList<int> indexes;
    QList<AnnotationGroup*> groups;
    AVItemL *av = static_cast<AVItemL*>(item);
    QString objName;
    if(av->type == AVItemType_Group) {
        AnnotationGroup *topGroup = (static_cast<AVGroupItemL*>(av))->getAnnotationGroup();
        int ind = findPosition(topGroup);
        itemMap[getRootGroupName(topGroup->getGObject())][ind] = 0;
    } else if(av->type == AVItemType_Annotation) {

        AnnotationGroup *topGroup = (static_cast<AVGroupItemL*>(av->parent()))->getAnnotationGroup();
        int offset = topGroup->getAnnotations().indexOf((static_cast<AVAnnotationItemL*>(av))->annotation);
        int ind = findPosition(topGroup) + offset + 1;
        itemMap[getRootGroupName(topGroup->getGObject())][ind] = 0;
    }
}

void TreeIndex::recalculate( AnnotationTableObject * obj ){
    std::vector<char> items = std::vector<char>(getChildNumber(obj), 0);
    itemMap.insert(getRootGroupName(obj), items);
}

int TreeIndex::getChildNumber( AnnotationTableObject * obj ){
    AnnotationGroup *rootGroup = obj->getRootGroup();
    int res = 1;
    foreach(AnnotationGroup *gr, rootGroup->getSubgroups()) {
        res += getChildNumber(gr);
    }
    return res;
}

void TreeIndex::deleteItem(Annotation *a, AnnotationGroup *gr) {
    int offset = gr->getAnnotations().indexOf(a);
    int ind = findPosition(gr) + offset + 1;
    itemMap[getRootGroupName(gr->getGObject())].erase(itemMap[getRootGroupName(gr->getGObject())].begin() + ind);
    if(gr->getAnnotations().isEmpty()) {
        ind = findPosition(gr);
        itemMap[getRootGroupName(gr->getGObject())][ind] = 0;  
    }
}

void TreeIndex::deleteItem(AnnotationGroup *gr) {
    int ind = findPosition(gr);
    QString name = gr->getGObject() ? getRootGroupName(gr->getGObject()) : getRootGroupName(gr->getParentGroup()->getGObject());
    itemMap[name].erase(itemMap[name].begin() + ind);
    if(gr->getParentGroup()->getSubgroups().isEmpty()) {
        ind = findPosition(gr->getParentGroup());
        itemMap[name][ind] = 0;
    }
}

void TreeIndex::addItem(Annotation *a, AnnotationGroup *gr) {
    int ind;
    if(gr->getAnnotations().indexOf(a) != 0) {
        int offset = gr->getAnnotations().indexOf(a);
        ind = findPosition(gr) + offset;
    } else {
        while(gr->getParentGroup()) {
            int i = gr->getParentGroup()->getSubgroups().indexOf(gr);
            if(i != 0) {
                gr = gr->getParentGroup()->getSubgroups()[i - 1];
                break;
            }
            gr = gr->getParentGroup();
        }
        ind = findPosition(gr);
    }
    itemMap[getRootGroupName(gr->getGObject())].insert(itemMap[getRootGroupName(gr->getGObject())].begin() + ind, 1, 0);
}

void TreeIndex::addItem(AnnotationGroup *gr) {
    int ind;
    while(gr->getParentGroup()) {
        int i = gr->getParentGroup()->getSubgroups().indexOf(gr);
        if(i != 0) {
            gr = gr->getParentGroup()->getSubgroups()[i - 1];
            break;
        }
        gr = gr->getParentGroup();
    }
    ind = findPosition(gr);
    itemMap[getRootGroupName(gr->getGObject())].insert(itemMap[getRootGroupName(gr->getGObject())].begin() + ind, 1, 0);
}

QString TreeIndex::getRootGroupName(AnnotationTableObject* aobj) const{
    AnnotationGroup *gr = aobj->getRootGroup();
    QString text = aobj->getGObjectName();
    if (aobj->getDocument() != NULL ) {
        QString docShortName = aobj->getDocument()->getName();
        assert(!docShortName.isEmpty());
        text = gr->getGObject()->getGObjectName() + " ["+docShortName+"]";
    }
    return text;
}


//////////////////////////////////////////////////////////////////////////
/// Tree model
bool AVItemL::processLinks(const QString& qName, const QString& qValue, int col) {
    bool linked = false;
    if (qName == "db_xref") {
        QStringList l = qValue.split(":");
        QString dbName = l[0];
        QString dbId = l.size() > 1 ? l[1] : "";
        DBXRefInfo info = AppContext::getDBXRefRegistry()->getRefByKey(dbName);
        linked = !info.url.isEmpty();
        setToolTip(col, info.comment);
        if (linked) {
            setData(col, Qt::UserRole, true);
        }
    } 

    if (linked) {
        QFont f = font(col);
        f.setUnderline(true);
        setFont(col, f);
        setForeground(col, Qt::blue);
    }
    return linked;
}

bool AVItemL::removeChildren(int position, int count)
{
    for (int row = 0; row < count; ++row) {
        delete takeChild(position);
    }
    return true;
}

bool AVItemL::addChildren(int position, int count, AVItemL *item)
{
    for (int row = 0; row < count; ++row) {
        if(indexOfChild(item) != -1) {
            takeChild(indexOfChild(item));
        }
        insertChild(position, item);
    }
    return true;
}

bool AVItemL::isColumnLinked(int col) const {
    return data(col, Qt::UserRole).type() == QVariant::Bool;
}

QString AVItemL::buildLinkURL(int col) const {
    assert(isColumnLinked(col));
    QString qValue = text(col);
    QStringList split = qValue.split(":");
    QString type = split.first();
    QString id = split.size() < 2 ? QString("") : split[1];
    QString url = AppContext::getDBXRefRegistry()->getRefByKey(type).url.arg(id);
    return url;
}

QString AVItemL::getFileUrl(int col) const {
    assert(isColumnLinked(col));
    QStringList split = text(col).split(":");
    QString type = split.first();
    QString fileUrl = AppContext::getDBXRefRegistry()->getRefByKey(type).fileUrl;
    if (!fileUrl.isEmpty()) {
        QString id = split.size() < 2 ? QString("") : split[1];
        return fileUrl.arg(id);
    }
    return fileUrl;

}

AVGroupItemL::AVGroupItemL(AnnotationsTreeViewL* _atv, AVGroupItemL* parent, AnnotationGroup* g) : AVItemL(parent, AVItemType_Group), group(g), atv(_atv) 
{
    updateVisual();
}

AVGroupItemL::~AVGroupItemL() {
    group = NULL;
}

const QIcon& AVGroupItemL::getGroupIcon() {
    static QIcon groupIcon(":/core/images/group_green_active.png");
    return groupIcon;
}

const QIcon& AVGroupItemL::getDocumentIcon() {
    static QIcon groupIcon(":/core/images/gobject.png");
    return groupIcon;
}

void AVGroupItemL::updateVisual() {
    if (parent() == NULL || parent()->parent() == NULL) { // document item
        AnnotationTableObject* aobj  = group->getGObject();
        Document* doc = aobj->getDocument();
        QString text = aobj->getGObjectName();
        if (doc != NULL ) {
            QString docShortName = aobj->getDocument()->getName();
            assert(!docShortName.isEmpty());
            text = group->getGObject()->getGObjectName() + " ["+docShortName+"]";
            if (aobj->isTreeItemModified()) { 
                text+=" *";
            }
        }
        setText(0, text);
        setIcon(0, getDocumentIcon());
        GUIUtils::setMutedLnF(this, aobj->getAnnotations().count() == 0, false);
    } else { // usual groups with annotations
        int na = group->getAnnotations().size();
        int ng = group->getSubgroups().size();
        QString nameString = group->getGroupName() + "  " + QString("(%1, %2)").arg(ng).arg(na);
        setText(0, nameString);
        setIcon(0, getGroupIcon());

        // if all child items are muted -> mute this group too
        bool showDisabled = childCount() > 0; //empty group is not disabled
        for (int i = 0; i < childCount(); i++) {
            QTreeWidgetItem* childItem = child(i);
            if (!GUIUtils::isMutedLnF(childItem))  {
                showDisabled = false;
                break;
            }
        }
        GUIUtils::setMutedLnF(this, showDisabled, false);
    }
}

void AVGroupItemL::updateAnnotations(const QString& nameFilter, ATVAnnUpdateFlags f) {
    bool noFilter = nameFilter.isEmpty();
    for (int j = 0; j < childCount(); j++) {
        AVItemL* item = static_cast<AVItemL*>(child(j));
        if (item->type == AVItemType_Group) {
            AVGroupItemL* level1 = static_cast<AVGroupItemL*>(item);
            if (noFilter || level1->group->getGroupName() == nameFilter) {
                level1->updateAnnotations(nameFilter, f);
            }
        } else {
            assert(item->type == AVItemType_Annotation);
            AVAnnotationItemL* aItem= static_cast<AVAnnotationItemL*>(item);
            if (noFilter || aItem->annotation->getAnnotationName() == nameFilter) {
                aItem->updateVisual(f);
            }
        }
    }
}

bool AVGroupItemL::isReadonly() const {
    //documents names are not editable
    return group->getParentGroup() == NULL ? true: group->getGObject()->isStateLocked();
}

void AVGroupItemL::findAnnotationItems(QList<AVAnnotationItemL*>& result, Annotation* a) const {
    for (int i = 0, n = childCount(); i < n; i++) {
        AVItemL* item = static_cast<AVItemL*>(child(i));
        if (item->type == AVItemType_Group) {
            AVGroupItemL* gi = static_cast<AVGroupItemL*>(item);
            gi->findAnnotationItems(result, a);
        } else if (item->type == AVItemType_Annotation) {
            AVAnnotationItemL* ai = static_cast<AVAnnotationItemL*>(item);
            if (ai->annotation == a) {
                result.append(ai);
            }
        }
    }
}

AnnotationTableObject* AVGroupItemL::getAnnotationTableObject() const {
    return group->getGObject();
}

AnnotationGroup* AVGroupItemL::getAnnotationGroup() const {
    return group;
}

int AVGroupItemL::childNumber() const{
    if(parent()) {
        if(parent()->parent() == NULL) {//root item
            const QTreeWidgetItem *root = parent();
            for(int i = 0; i < root->childCount();i++) {
                if(root->child(i) == this) {
                    return i;
                }
            }
            return 0;
        }
        return parent()->indexOfChild((QTreeWidgetItem*)this);
    } else {
        return 0;
    }
}
/*
void AVGroupItemL::changeIndexes(int i) {
    for(int j = i; j < childCount(); j++) {
        AVItemL *childItem = static_cast<AVItemL*>(child(j));
        //childItem->setRow(j);
    }
}*/

AVAnnotationItemL::AVAnnotationItemL(AVGroupItemL* parent, Annotation* a) : AVItemL(parent, AVItemType_Annotation), annotation(a) {
    updateVisual(ATVAnnUpdateFlags(ATVAnnUpdateFlag_BaseColumns) | ATVAnnUpdateFlag_QualColumns);    
    hasNumericQColumns = false;
}

AVAnnotationItemL::~AVAnnotationItemL() {
    annotation = NULL;
}

#define MAX_ICONS_CACHE_SIZE 500

QMap<QString, QIcon>& AVAnnotationItemL::getIconsCache() {
    static QMap<QString, QIcon> iconsCache;
    return iconsCache;
}

void AVAnnotationItemL::updateVisual(ATVAnnUpdateFlags f) {

    const QString& name = annotation->getAnnotationName();
    const AnnotationSettings* as = AppContext::getAnnotationsSettingsRegistry()->getAnnotationSettings(name);

    if (f.testFlag(ATVAnnUpdateFlag_BaseColumns)) {
        QMap<QString, QIcon>& cache = getIconsCache();
        QIcon icon = cache.value(name);
        if (icon.isNull()) {
            QColor iconColor = as->visible ? as->color : Qt::lightGray;
            icon = GUIUtils::createSquareIcon(iconColor, 9);
            if (cache.size() > MAX_ICONS_CACHE_SIZE) {
                cache.clear();
            }
            cache[name] = icon;
        }
        assert(!icon.isNull());

        setIcon(0, icon);
        setText(0, annotation->getAnnotationName());
        locationString = Genbank::LocationParser::buildLocationString(annotation->data());
        setText(1, locationString);
    }

    if (f.testFlag(ATVAnnUpdateFlag_QualColumns)) {
        //setup custom qualifiers columns
        AnnotationsTreeViewL* atv = getAnnotationTreeView();
        assert(atv!=NULL);
        const QStringList& colNames = atv->getQualifierColumnNames();
        hasNumericQColumns = false;
        for (int i=0, n = colNames.size(); i < n ;i++) {
            int col = 2+i;
            QString colName = colNames[i];
            QString colText = annotation->findFirstQualifierValue(colName);
            setText(2+i, colText);
            bool linked = processLinks(colName, colText,  col);
            if (!linked) {
                bool ok  = false;
                double d = colText.toDouble(&ok);
                if (ok) {
                    setData(col, Qt::UserRole, d);
                    hasNumericQColumns = true;
                }
            }
        }
    }

    GUIUtils::setMutedLnF(this, !as->visible, true);
}

QVariant AVAnnotationItemL::data( int col, int role ) const {
    if (col == 1 && role == Qt::DisplayRole) {
        if (locationString.isEmpty()) {
            locationString = Genbank::LocationParser::buildLocationString(annotation->data());
        }
        return locationString;
    }

    return QTreeWidgetItem::data(col, role);
}

int AVAnnotationItemL::childNumber() const {
    return parent()->indexOfChild((QTreeWidgetItem*)this);
}

bool AVAnnotationItemL::operator<(const QTreeWidgetItem & other) const {
    int col = treeWidget()->sortColumn();
    const AVItemL& avItem = (const AVItemL&)other;
    if (avItem.type != AVItemType_Annotation) {
        return text(col) < other.text(col);
    }
    const AVAnnotationItemL& ai = (const AVAnnotationItemL&)other;
    if (col == 0) {
        QString name1 = annotation->getAnnotationName();
        QString name2 = ai.annotation->getAnnotationName();
        if (name1 == name2) { 
            // for annotations with equal names we compare locations
            // this allows to avoid resorting on lazy qualifier loading
            return annotation->getLocation()->regions[0] < ai.annotation->getLocation()->regions[0];
        }
        return name1 < name2;
    }
    if (col == 1 || (isColumnNumeric(col) && ai.isColumnNumeric(col))) {
        double oval = ai.getNumericVal(col);
        double mval = getNumericVal(col);
        return mval < oval;
    }
    return text(col) < other.text(col);
}

bool AVAnnotationItemL::isColumnNumeric(int col) const {
    if (col == 0) {
        return false;
    } 
    if (col == 1) {
        return true;
    }
    if (!hasNumericQColumns) {
        return false;
    }
    return data(col, Qt::UserRole).type() == QVariant::Double;
}

double AVAnnotationItemL::getNumericVal(int col) const {
    if (col == 1) {
        const U2Region& r = annotation->getLocation()->regions[0];
        return r.startPos;
    }
    bool ok  = false;
    double d = data(col, Qt::UserRole).toDouble(&ok);
    assert(ok);
    return d;
}

void AVAnnotationItemL::removeQualifier(const U2Qualifier& q) {
    for(int i=0, n = childCount(); i < n; i++) {
        AVQualifierItemL* qi = static_cast<AVQualifierItemL*>(child(i));
        if (qi->qName == q.name && qi->qValue == q.value) {
            delete qi;
            break;
        }
    }
    updateVisual(ATVAnnUpdateFlag_QualColumns);
}

void AVAnnotationItemL::addQualifier(const U2Qualifier& q) {
    AVQualifierItemL* qi = new AVQualifierItemL(this, q); Q_UNUSED(qi);
    updateVisual(ATVAnnUpdateFlag_QualColumns);
}


AVQualifierItemL* AVAnnotationItemL::findQualifierItem(const QString& name, const QString& val) const {
    for(int i=0, n = childCount(); i < n; i++) {
        AVQualifierItemL* qi = static_cast<AVQualifierItemL*>(child(i));
        if (qi->qName == name && qi->qValue == val) {
            return qi;
        }
    }
    return NULL;
}

AVQualifierItemL::AVQualifierItemL(AVAnnotationItemL* parent, const U2Qualifier& q) 
: AVItemL(parent, AVItemType_Qualifier), qName(q.name), qValue(q.value)
{
    setText(0, qName);
    setText(1, qValue);

    processLinks(qName, qValue, 1);
}

int AVQualifierItemL::childNumber() const {
    return parent()->indexOfChild((QTreeWidgetItem*)this);
}


}