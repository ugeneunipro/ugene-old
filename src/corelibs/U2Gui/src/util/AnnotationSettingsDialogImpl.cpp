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

#include "AnnotationSettingsDialogImpl.h"
#include "AnnotationSettingsDialogController.h"

#include <U2Core/GObjectTypes.h>
#include <U2Core/AnnotationTableObject.h>

#include <U2Gui/TextEditorDialog.h>

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentModel.h>

#include <U2Gui/GUIUtils.h>

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QIcon>
#include <QtGui/QColorDialog>

namespace U2 {

#define NAME_COLUMN         0
#define COLOR_COLUMN        1
#define AMINO_COLUMN        2
#define HIGHLIGHT_COLUMN    3
#define QUALIFIER_COLUMN    4

void AnnotationSettingsDialogController::run(QWidget* p) {
    AnnotationSettingsDialogImpl d(p);
    d.exec();
}


AnnotationSettingsDialogImpl::AnnotationSettingsDialogImpl(QWidget* p)
: QDialog(p) 
{
    setupUi(this);
    tree->headerItem()->setToolTip(NAME_COLUMN, tr("Annotation name"));
    tree->headerItem()->setToolTip(COLOR_COLUMN, tr("A color used to highlight annotation"));
    tree->headerItem()->setToolTip(AMINO_COLUMN, tr("If checked the annotation will be shown on amino strand"));
    tree->headerItem()->setToolTip(HIGHLIGHT_COLUMN, tr("If unchecked the annotation will not be highlighted with color"));
    tree->headerItem()->setToolTip(QUALIFIER_COLUMN, tr("A comma-separated list of qualifier names.<br> The value of qualifiers will be shown instead of the annotation names in PanView"));

    connect(tree, SIGNAL(itemClicked(QTreeWidgetItem*, int)), SLOT(sl_itemClicked(QTreeWidgetItem*, int)));
    connect(tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), SLOT(sl_itemDoubleClicked(QTreeWidgetItem*, int)));
    connect(okButton, SIGNAL(clicked()), SLOT(sl_okClicked()));
    loadModel();
}

/*
static void findAllAnnotationsNamesInProject(QSet<QString>& allAnnotations)  {
    Project* p = AppContext::getProject();
    foreach(Document* doc , p->getDocuments()) {
        foreach(GObject* obj, doc->getObjects()) {
            if (obj->getGObjectType() == GObjectTypes::ANNOTATION_TABLE) {
                AnnotationTableObject* ao = qobject_cast<AnnotationTableObject*>(obj);
                foreach(Annotation* a, ao->getAnnotations()) {
                    allAnnotations.insert(a->getAnnotationName());
                }
            }
        }
    }
}
*/

static void findAllAnnotationsNamesInSettings(QSet<QString>& allAnnotations) {
    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
    foreach(const QString& s, asr->getAllSettings()) {
        allAnnotations.insert(s);
    }
}

static bool caseInsensitiveLessThan(const QString &s1, const QString &s2) {
    return s1.toLower() < s2.toLower();
}


void AnnotationSettingsDialogImpl::loadModel() {
    QSet<QString> allAnnotations;
    //findAllAnnotationsNamesInProject(allAnnotations);
    findAllAnnotationsNamesInSettings(allAnnotations);
    
    QStringList sortedList = allAnnotations.toList();
    qSort(sortedList.begin(), sortedList.end(), caseInsensitiveLessThan);
    
    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
    foreach(const QString& name, sortedList) {
        AnnotationSettings* as = asr->getAnnotationSettings(name);
        ASTreeItem* item = new ASTreeItem(as);
        tree->addTopLevelItem(item);    
    }
}


void AnnotationSettingsDialogImpl::storeModel() {
    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
    QList<AnnotationSettings*> mods;
    for (int i=0; i<tree->topLevelItemCount(); i++) {
        ASTreeItem *item = static_cast<ASTreeItem*>(tree->topLevelItem(i));
        item->fillModel();
        const AnnotationSettings& s = item->as;
        AnnotationSettings* orig = asr->getAnnotationSettings(s.name);
        if (!s.equals(orig)) {
            orig->amino = s.amino;
            orig->visible = s.visible;
            orig->color = s.color;
            orig->nameQuals = s.nameQuals;
            mods.append(orig);
        }
    }
    if (!mods.isEmpty()) {
        asr->changeSettings(mods, true);
    }
}

void AnnotationSettingsDialogImpl::sl_okClicked() {
    storeModel();
    accept();
}


void AnnotationSettingsDialogImpl::sl_itemClicked(QTreeWidgetItem * i, int column) {
    if (column != 1) {
        return;
    }
    ASTreeItem* item = static_cast<ASTreeItem*>(i);
    QColor c = QColorDialog::getColor(item->as.color, this);
    item->as.color = c;
    item->drawColorCell();
}

void AnnotationSettingsDialogImpl::sl_itemDoubleClicked(QTreeWidgetItem* item, int col) {
    if (col!=QUALIFIER_COLUMN) {
        return;
    }
    ASTreeItem* ai = static_cast<ASTreeItem*>(item);
    //TODO controller
    TextEditorDialog d(this, tr("Visual qualifiers for %1").arg(ai->as.name), 
        tr("Please enter a comma-separated list of qualifier names."
        "<br> <i>The value of qualifiers (with list priority) will be shown instead of the annotation names in PanView</i>"), 
        ai->as.nameQuals.join(","), true);
    int rc = d.exec();
    if (rc == QDialog::Rejected){
        return;
    }
    QString text = d.getText().simplified().replace(" ", "");
    if (text.isEmpty()) {
        ai->as.nameQuals.clear();    
    } else {
        ai->as.nameQuals = text.split(',', QString::SkipEmptyParts);    
    }
    ai->setText(QUALIFIER_COLUMN, ai->as.nameQuals.join(","));
}

//////////////////////////////////////////////////////////////////////////

ASTreeItem::ASTreeItem(const AnnotationSettings* _as) : as(*_as) {
    setText(NAME_COLUMN, as.name);
    drawColorCell();
    setCheckState(AMINO_COLUMN, as.amino ? Qt::Checked : Qt::Unchecked);
    setCheckState(HIGHLIGHT_COLUMN, as.visible ? Qt::Checked : Qt::Unchecked);
    QString qNames = as.nameQuals.join(",");
    setText(QUALIFIER_COLUMN, qNames);
    setToolTip(QUALIFIER_COLUMN, qNames);
}

void ASTreeItem::fillModel() {
    as.amino = checkState(AMINO_COLUMN) == Qt::Checked; 
    as.visible = checkState(HIGHLIGHT_COLUMN) == Qt::Checked;
}

void ASTreeItem::drawColorCell() {
    QIcon icon = GUIUtils::createSquareIcon(as.color, 14);
    setIcon(COLOR_COLUMN, icon);
}

}//namespace
