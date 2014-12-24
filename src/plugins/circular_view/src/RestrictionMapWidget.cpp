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

#include <qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QVBoxLayout>
#else
#include <QtWidgets/QVBoxLayout>
#endif

#include <U2Core/Annotation.h>
#include <U2Core/AnnotationGroup.h>
#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AutoAnnotationsSupport.h>
#include <U2Core/AnnotationSelection.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2Algorithm/EnzymeModel.h>
#include <U2Formats/GenbankLocationParser.h>

#include "RestrictionMapWidget.h"

#define     ENZYME_FOLDER_ITEM_TYPE     1022
#define     ENZYME_ITEM_TYPE            1023

namespace U2 {

//////////////////////////////////////////////////////////////////////////
///EnzymeItem

EnzymeItem::EnzymeItem( const QString& location, const Annotation &a )
: QTreeWidgetItem(QStringList(location), ENZYME_ITEM_TYPE), annotation(a)
{

}


//////////////////////////////////////////////////////////////////////////
///EnzymeFolderItem

EnzymeFolderItem::EnzymeFolderItem( const QString& name )
: QTreeWidgetItem(ENZYME_FOLDER_ITEM_TYPE), enzymeName(name)
{
    setText(0, QString("%1 : %2 %3").arg(name).arg(0).arg("sites"));
}
void EnzymeFolderItem::addEnzymeItem( const Annotation &enzAnn )
{
    const AnnotationData data = enzAnn.getData( );
    QString location = Genbank::LocationParser::buildLocationString( &data );
    addChild(new EnzymeItem(location, enzAnn) );
    setIcon(0, QIcon(":circular_view/images/folder.png"));
    int count = childCount();
    QString site = count == 1 ? RestrctionMapWidget::tr("site") : RestrctionMapWidget::tr("sites");
    setText(0, QString("%1 : %2 %3").arg(getName()).arg(count).arg(site));
}

void EnzymeFolderItem::removeEnzymeItem( const Annotation &enzAnn )
{
    int count = childCount();
    for (int i = 0; i < count; ++i) {
        EnzymeItem* item = static_cast<EnzymeItem*> ( child(i) );
        if (item->getEnzymeAnnotation() == enzAnn) {
            removeChild(item);
            QString site = --count == 1 ? RestrctionMapWidget::tr("site") : RestrctionMapWidget::tr("sites");
            setText(0, QString("%1 : %2 %3").arg(getName()).arg(count).arg(site));
            if (count == 0) {
                setIcon(0, QIcon(":circular_view/images/empty_folder.png"));
            }
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
///RestrictionMapWidget

RestrctionMapWidget::RestrctionMapWidget( ADVSequenceObjectContext* context, QWidget *p )
: QWidget(p), ctx(context)
{
    assert(context != NULL);
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    treeWidget = new QTreeWidget(this);
    treeWidget->setObjectName("restrictionMapTreeWidget");
    treeWidget->setColumnCount(1);
    treeWidget->setHeaderLabel(tr("Restriction Sites Map"));
    connect(treeWidget,SIGNAL(itemSelectionChanged()), SLOT(sl_itemSelectionChanged()));

    layout->addWidget(treeWidget);

    registerAnnotationObjects();
    updateTreeWidget();
    initTreeWidget();

}


void RestrctionMapWidget::updateTreeWidget()
{
    treeWidget->clear();

    QString selection = AppContext::getSettings()->getValue(EnzymeSettings::LAST_SELECTION).toString();
    if (selection.isEmpty()) {
        selection = EnzymeSettings::COMMON_ENZYMES;
    }
    QStringList selectedEnzymes = selection.split(ENZYME_LIST_SEPARATOR, QString::SkipEmptyParts);

    QList<QTreeWidgetItem*> items;
    foreach(const QString& enzyme, selectedEnzymes) {
        EnzymeFolderItem* item = new EnzymeFolderItem( enzyme );
        item->setIcon(0, QIcon(":circular_view/images/empty_folder.png"));
        items.append(item);
    }
    treeWidget->insertTopLevelItems(0, items);
    treeWidget->sortItems(0, Qt::AscendingOrder);
}

void RestrctionMapWidget::registerAnnotationObjects()
{
    QSet<AnnotationTableObject *> aObjs = ctx->getAnnotationObjects(true);
    foreach ( const AnnotationTableObject *ao, aObjs ) {
        connect(ao, SIGNAL(si_onAnnotationsAdded(const QList<Annotation> &)), SLOT(sl_onAnnotationsAdded(const QList<Annotation> &)));
        connect(ao, SIGNAL(si_onAnnotationsRemoved(const QList<Annotation> &)), SLOT(sl_onAnnotationsRemoved(const QList<Annotation> &)));
        connect(ao, SIGNAL(si_onAnnotationsInGroupRemoved(const QList<Annotation> &, const AnnotationGroup )),
            SLOT(sl_onAnnotationsInGroupRemoved(const QList<Annotation> &, const AnnotationGroup &)));
        connect(ao, SIGNAL(si_onGroupCreated( const AnnotationGroup & )), SLOT(sl_onAnnotationsGroupCreated( const AnnotationGroup &)));
    }
}

void RestrctionMapWidget::sl_onAnnotationsAdded( const QList<Annotation> &anns ) {
    foreach ( const Annotation &a, anns ) {
        QString aName = a.getName();
        EnzymeFolderItem* folderItem = findEnzymeFolderByName(aName);
        if (folderItem) {
               folderItem->addEnzymeItem(a);
        }
    }

    //TODO: enable "intelligent" sorting by reimplementing custom AbstractModel
    // Take into account number of items in each enzymes folder
    treeWidget->sortItems(0, Qt::AscendingOrder);
}

void RestrctionMapWidget::sl_onAnnotationsRemoved( const QList<Annotation> &anns ) {
    foreach ( const Annotation &a, anns) {
        EnzymeFolderItem* folderItem = findEnzymeFolderByName(a.getName());
        if (folderItem) {
            folderItem->removeEnzymeItem(a);
        }
    }
}

EnzymeFolderItem* RestrctionMapWidget::findEnzymeFolderByName( const QString& enzymeName )
{
    int count = treeWidget->topLevelItemCount();

    for (int i = 0; i < count; i++) {
        assert(treeWidget->topLevelItem(i)->type() == ENZYME_FOLDER_ITEM_TYPE);
        EnzymeFolderItem* item = static_cast<EnzymeFolderItem*> ( treeWidget->topLevelItem(i) );
        if (item->getName() == enzymeName) {
            return item;
        }
    }

    return NULL;
}

void RestrctionMapWidget::sl_itemSelectionChanged()
{
    QList<QTreeWidgetItem*> selected = treeWidget->selectedItems();
    foreach (QTreeWidgetItem* item, selected) {
        if (item->type() == ENZYME_ITEM_TYPE) {
            EnzymeItem* enzItem = static_cast<EnzymeItem*> (item);
            AnnotationSelection* sel = ctx->getAnnotationsSelection();
            sel->clear();
            sel->addToSelection( enzItem->getEnzymeAnnotation() );
        }
    }

}

void RestrctionMapWidget::sl_onAnnotationsGroupCreated( const AnnotationGroup &g )
{
    if (g.getName() == ANNOTATION_GROUP_ENZYME) {
        updateTreeWidget();
    }
}

void RestrctionMapWidget::initTreeWidget()
{
    QSet<AnnotationTableObject *> aObjs = ctx->getAnnotationObjects(true);
    foreach (AnnotationTableObject *obj, aObjs) {
        QList<Annotation> anns = obj->getAnnotations();
        foreach ( const Annotation &a, anns ) {
            QString aName = a.getName();
            EnzymeFolderItem *folderItem = findEnzymeFolderByName(aName);
            if (folderItem) {
                folderItem->addEnzymeItem(a);
            }
        }
    }
    treeWidget->sortItems(0, Qt::AscendingOrder);
}

void RestrctionMapWidget::sl_onAnnotationsInGroupRemoved( const QList<Annotation> &anns, const AnnotationGroup &group )
{
    if (group.getName() == ANNOTATION_GROUP_ENZYME) {
        foreach ( const Annotation &a, anns ) {
            EnzymeFolderItem* folderItem = findEnzymeFolderByName( a.getName());
            if (folderItem) {
                folderItem->removeEnzymeItem(a);
            }
        }
    }
}


}//namespace
