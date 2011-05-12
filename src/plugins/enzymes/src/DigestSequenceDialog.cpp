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
#include <U2Core/AutoAnnotationsSupport.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/U2SafePoints.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2Gui/CreateAnnotationWidgetController.h>

#include "EnzymesIO.h"
#include "EnzymesQuery.h"
#include "CloningUtilTasks.h"
#include "FindEnzymesDialog.h"
#include "DigestSequenceDialog.h"

#include <memory>

namespace U2 {

const QString DigestSequenceDialog::WAIT_MESSAGE(tr("The restrictions sites are being updated. Please wait"));
const QString DigestSequenceDialog::HINT_MESSAGE(tr("Hint: there are no available enzymes. Use \"Analyze->Find Restrictions Sites\" feature to find them."));


DigestSequenceDialog::DigestSequenceDialog( ADVSequenceObjectContext* ctx, QWidget* p )
: QDialog(p),seqCtx(ctx), timer(NULL), animationCounter(0)
{
    setupUi(this);
        
    dnaObj = qobject_cast<DNASequenceObject*>(ctx->getSequenceGObject());
    sourceObj = NULL;
    assert(dnaObj != NULL);
    
    addAnnotationWidget();
    searchForAnnotatedEnzymes(ctx);
    
    availableEnzymeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    selectedEnzymeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(addButton, SIGNAL(clicked()), SLOT(sl_addPushButtonClicked()));
    connect(addAllButton, SIGNAL(clicked()), SLOT(sl_addAllPushButtonClicked()));
    connect(removeButton, SIGNAL(clicked()), SLOT(sl_removePushButtonClicked()));
    connect(clearButton, SIGNAL(clicked()), SLOT(sl_clearPushButtonClicked()));
    
    updateAvailableEnzymeWidget();
    seqNameLabel->setText(dnaObj->getGObjectName());

    QList<Task*> topLevelTasks = AppContext::getTaskScheduler()->getTopLevelTasks();
    foreach(Task* t, topLevelTasks) {
        if (t->getTaskName() == AutoAnnotationsUpdateTask::NAME) {
            connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskStateChanged()));
            hintLabel->setText(WAIT_MESSAGE);
            animationCounter = 0;
            setUiEnabled(false);
            timer = new QTimer();
            connect(timer, SIGNAL(timeout()), SLOT(sl_timerUpdate()));
            timer->start(400);
        }
    }
        

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
    
    DigestSequenceTask* task = new DigestSequenceTask(dnaObj, sourceObj, aObj, resultEnzymes);

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

    bool empty = availableEnzymes.isEmpty();
    setUiEnabled(!empty);
    if (empty) {
        hintLabel->setText(HINT_MESSAGE);
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

void DigestSequenceDialog::sl_timerUpdate()
{
    const int MAX_COUNT = 5;
    animationCounter++;
    if (animationCounter > MAX_COUNT) {
        animationCounter = 1;
    }
    
    QString dots;
    dots.fill('.', animationCounter);
    hintLabel->setText(WAIT_MESSAGE + dots);


}

void DigestSequenceDialog::sl_taskStateChanged()
{
    Task* task = qobject_cast<Task*> ( sender() );
    SAFE_POINT(task != NULL, tr("Auto-annotations update task is NULL."),);
    
    if (task->getState() == Task::State_Finished) {
        timer->stop();
        hintLabel->setText(QString());
        searchForAnnotatedEnzymes(seqCtx);
        updateAvailableEnzymeWidget();
    }
}

void DigestSequenceDialog::setUiEnabled( bool enabled )
{
    okButton->setEnabled(enabled);
    addButton->setEnabled(enabled);
    addAllButton->setEnabled(enabled);
    removeButton->setEnabled(enabled);
    clearButton->setEnabled(enabled);
}







} // U2
