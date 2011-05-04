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

#include <QtGui/QMessageBox>

#include <U2Core/Log.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2Gui/CreateAnnotationWidgetController.h>

#include "EnzymesIO.h"
#include "EnzymesQuery.h"
#include "CloningUtilTasks.h"
#include "FindEnzymesDialog.h"
#include "DigestSequenceDialog.h"

#include <memory>

namespace U2 {

DigestSequenceDialog::DigestSequenceDialog( ADVSequenceObjectContext* ctx, QWidget* p )
: QDialog(p),seqCtx(ctx)
{
    setupUi(this);
        
    dnaObj = qobject_cast<DNASequenceObject*>(ctx->getSequenceGObject());
    sourceObj = NULL;
    assert(dnaObj != NULL);
    
    addAnnotationWidget();
    searchForAnnotatedEnzymes(ctx);
    //if (annotatedEnzymes.isEmpty()) {
    //    searchForEnzymesRadioButton->setChecked(true);
    //    useExistingRadioButton->setEnabled(false);
    //}
    
    availableEnzymeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    selectedEnzymeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(addButton, SIGNAL(clicked()), SLOT(sl_addPushButtonClicked()));
    connect(addAllButton, SIGNAL(clicked()), SLOT(sl_addAllPushButtonClicked()));
    connect(removeButton, SIGNAL(clicked()), SLOT(sl_removePushButtonClicked()));
    connect(clearButton, SIGNAL(clicked()), SLOT(sl_clearPushButtonClicked()));
//    connect(searchSettingsButton, SIGNAL(clicked()), SLOT(sl_searchSettingsPushButtonClicked()));
//    connect(useExistingRadioButton, SIGNAL(toggled(bool)), SLOT(sl_useAnnotatedRegionsSelected(bool)));
    
    updateAvailableEnzymeWidget();
    seqNameLabel->setText(dnaObj->getGObjectName());
    


}

QList<SEnzymeData> DigestSequenceDialog::findEnzymeDataById( const QString& id )
{
    QList<SEnzymeData> result;
    foreach (const SEnzymeData& enzyme, enzymesBase) {
        if (enzyme->id == id) {
            result.append(enzyme);
            break;
        }
    }
    return result;
}




void DigestSequenceDialog::accept()
{

    if (selectedEnzymes.isEmpty()) {
        QMessageBox::information(this, windowTitle(), tr("No enzymes are selected! Please select enzymes."));
        return;        
    }
    
    bool ok = loadEnzymesFile();
    if (!ok) {
        QMessageBox::critical(this, windowTitle(), tr("Cannot load enzymes library"));
        QDialog::reject();
    }

    QList<SEnzymeData> resultEnzymes;

    foreach (const QString& enzymeId, selectedEnzymes) {
        QList<SEnzymeData> foundEnzymes = findEnzymeDataById(enzymeId);
        resultEnzymes += foundEnzymes;
    }

    QString err = ac->validate();
    if (!err.isEmpty()) {
        QMessageBox::information(this, windowTitle(), err);
        return;
    }

    ac->prepareAnnotationObject();
    const CreateAnnotationModel& m = ac->getModel();
    AnnotationTableObject* aObj = m.getAnnotationObject();
    assert(aObj != NULL);
    
    DigestSequenceTask* task = NULL;
    //if (searchForEnzymesRadioButton->isChecked()) {
    //    task = new DigestSequenceTask(dnaObj, aObj, resultEnzymes); 
    //} else {
    //    task = new DigestSequenceTask(dnaObj, sourceObj, aObj, resultEnzymes);
    //}

    task = new DigestSequenceTask(dnaObj, sourceObj, aObj, resultEnzymes);

    AppContext::getTaskScheduler()->registerTopLevelTask(task);
    
    QDialog::accept();
}


void DigestSequenceDialog::addAnnotationWidget()
{
    CreateAnnotationModel acm;

    acm.sequenceObjectRef = GObjectReference(dnaObj);
    acm.hideAnnotationName = true;
    acm.hideLocation = true;
    acm.sequenceLen = dnaObj->getSequenceLen();
    acm.data->name = ANNOTATION_GROUP_FRAGMENTS;
    ac = new CreateAnnotationWidgetController(acm, this);
    QWidget* caw = ac->getWidget();    
    QVBoxLayout* l = new QVBoxLayout(this);
    l->setMargin(0);
    l->addWidget(caw);
    annotationsArea->setLayout(l);
    annotationsArea->setMinimumSize(caw->layout()->minimumSize());

}

void DigestSequenceDialog::searchForAnnotatedEnzymes(ADVSequenceObjectContext* ctx)
{
    QSet<AnnotationTableObject*> relatedAnns = ctx->getAnnotationObjects(true);

    foreach (AnnotationTableObject* a, relatedAnns ) {
        AnnotationGroup* grp = a->getRootGroup()->getSubgroup(ANNOTATION_GROUP_ENZYME, false);
        if (grp == NULL) {
            continue;
        }
        sourceObj = a;
        QSet<Annotation*> reSites;
        grp->findAllAnnotationsInGroupSubTree(reSites);
        foreach (Annotation* a, reSites) {
            QString enzymeId = a->getAnnotationName();
            annotatedEnzymes.insertMulti(enzymeId, a->getRegions().first() );
            availableEnzymes.insert(enzymeId);
        }

    }
    
}

void DigestSequenceDialog::updateAvailableEnzymeWidget()
{
    availableEnzymeWidget->clear();
    
    QList<QString> enzymesList(availableEnzymes.values());
    qSort(enzymesList);

    foreach ( const QString& enzymeId, enzymesList) {
        QString cutInfo;
        if (annotatedEnzymes.contains(enzymeId)) {
            int numCuts = annotatedEnzymes.values(enzymeId).count();
            cutInfo = QString(tr(" : %1 cut(s)")).arg(numCuts);
        }
        availableEnzymeWidget->addItem(enzymeId + cutInfo);
    }
}


void DigestSequenceDialog::updateSelectedEnzymeWidget()
{
    selectedEnzymeWidget->clear();

    foreach ( const QString& enzymeId, selectedEnzymes) {
        selectedEnzymeWidget->addItem(enzymeId);
    }
}


void DigestSequenceDialog::sl_addPushButtonClicked()
{
    QList<QListWidgetItem*> items = availableEnzymeWidget->selectedItems();
    foreach (QListWidgetItem* item, items) {
        QString enzymeId = item->text().split(":").first().trimmed();
        selectedEnzymes.insert(enzymeId);
    }
    
    updateSelectedEnzymeWidget();

}

void DigestSequenceDialog::sl_addAllPushButtonClicked()
{
    int itemCount = availableEnzymeWidget->count();
    for (int row = 0; row < itemCount; ++row) {
        QListWidgetItem* item = availableEnzymeWidget->item(row);
        QString enzymeId = item->text().split(":").first().trimmed();
        selectedEnzymes.insert(enzymeId);
    }

    updateSelectedEnzymeWidget();
}

void DigestSequenceDialog::sl_removePushButtonClicked()
{
    QList<QListWidgetItem*> items = selectedEnzymeWidget->selectedItems();
    foreach (QListWidgetItem* item, items) {
        selectedEnzymes.remove(item->text());
    }
    updateSelectedEnzymeWidget();
}
   

void DigestSequenceDialog::sl_clearPushButtonClicked()
{
   selectedEnzymes.clear();
   updateSelectedEnzymeWidget();
}

void DigestSequenceDialog::sl_searchSettingsPushButtonClicked()
{
    std::auto_ptr<QDialog> dlg( enzymesSelectorHandler.createSelectorDialog() );
    int rt = dlg->exec();
    if (rt == QDialog::Accepted) {
        availableEnzymes.clear();
        QStringList enzymes = enzymesSelectorHandler.getSelectedString(dlg.get()).split(",");
        foreach (const QString& enzymeId, enzymes) {
            availableEnzymes.insert(enzymeId);
        }
    }
    updateAvailableEnzymeWidget();
}

bool DigestSequenceDialog::loadEnzymesFile( )
{
    TaskStateInfo ti;
   
    QString dataDir = QDir::searchPaths( PATH_PREFIX_DATA ).first() + "/enzymes/";
    QString url = dataDir + DEFAULT_ENZYMES_FILE;
    
    if (!QFileInfo(url).exists()) {
        ti.setError(  tr("File not exists: %1").arg(url) );
    } else {
        enzymesBase = EnzymesIO::readEnzymes(url, ti);
    }
    if (ti.hasError()) {
       ioLog.error(QString("Failed loading enzymes file: %1").arg(url));
       return false;
    }

    return true;
}

void DigestSequenceDialog::sl_useAnnotatedRegionsSelected(bool toggle)
{
    availableEnzymes.clear();
    annotatedEnzymes.clear();
    if (toggle == true) {
        searchForAnnotatedEnzymes(seqCtx);
    }
    updateAvailableEnzymeWidget();

}






} // U2
