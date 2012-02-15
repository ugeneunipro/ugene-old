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

#include "DotPlotDialog.h"
#include "DotPlotTasks.h"

#include <U2Core/global.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/DNAAlphabet.h>

#include <U2Gui/CreateAnnotationWidgetController.h>
#include <U2Gui/LastUsedDirHelper.h>
#include <U2Gui/DialogUtils.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>

#include <QtGui/QFileDialog>
#include <QtGui/QColorDialog>

namespace U2 {

DotPlotDialog::DotPlotDialog(QWidget *parent, AnnotatedDNAView* currentADV, int minLen, int identity, 
                             ADVSequenceObjectContext *sequenceX, ADVSequenceObjectContext *sequenceY, 
                             bool dir, bool inv, const QColor &dColor, const QColor &iColor, bool hideLoadSequences)
: QDialog(parent), xSeq(sequenceX), ySeq(sequenceY), adv(currentADV), directColor(dColor), invertedColor(iColor)
,openSequenceTask(NULL), curURL("")
{
    setupUi(this);

    SAFE_POINT(adv != NULL, "DotPlotDialog called without view context!", );
    
    directCheckBox->setChecked(dir);
    invertedCheckBox->setChecked(inv);

    updateColors();

    // set algorithms
    algoCombo->addItem(tr("Auto"), RFAlgorithm_Auto);
    algoCombo->addItem(tr("Suffix index"), RFAlgorithm_Suffix);
    algoCombo->addItem(tr("Diagonals"), RFAlgorithm_Diagonal);

    int xSeqIndex = -1, ySeqIndex = -1, curIndex = 0;

    //sequences in the project
    QList<GObject*> allSequences  = GObjectUtils::findAllObjects(UOF_LoadedOnly, GObjectTypes::SEQUENCE);
    foreach (GObject* obj, allSequences) {
        U2SequenceObject* seqObj = qobject_cast<U2SequenceObject*>(obj);
        QString name = seqObj->getGObjectName();
        
        xAxisCombo->addItem(name);
        yAxisCombo->addItem(name);

        if (sequenceX && (sequenceX->getSequenceGObject() == seqObj)) {
            xSeqIndex = curIndex;
        }
        if (sequenceY && (sequenceY->getSequenceGObject() == seqObj)) {
            ySeqIndex = curIndex;
        }
        curIndex++;
        sequences << seqObj;
    }

    if (xSeqIndex >= 0) {
        xAxisCombo->setCurrentIndex(xSeqIndex);
    }
    if (ySeqIndex >= 0) {
        yAxisCombo->setCurrentIndex(ySeqIndex);
    } else if (sequences.size() > 1) {    // choose the second sequence for Y axis by default
        yAxisCombo->setCurrentIndex(1);
    }
   
    minLenBox->setValue(minLen);
    identityBox->setValue(identity);

    connect(minLenHeuristicsButton, SIGNAL(clicked()), SLOT(sl_minLenHeuristics()));
    connect(hundredPercentButton, SIGNAL(clicked()), SLOT(sl_hundredPercent()));

    connect(directCheckBox, SIGNAL(clicked()), SLOT(sl_directInvertedCheckBox()));
    connect(invertedCheckBox, SIGNAL(clicked()), SLOT(sl_directInvertedCheckBox()));

    connect(directColorButton, SIGNAL(clicked()), SLOT(sl_directColorButton()));
    connect(invertedColorButton, SIGNAL(clicked()), SLOT(sl_invertedColorButton()));

    connect(directDefaultColorButton, SIGNAL(clicked()), SLOT(sl_directDefaultColorButton()));
    connect(invertedDefaultColorButton, SIGNAL(clicked()), SLOT(sl_invertedDefaultColorButton()));

    connect(loadSequenceButton, SIGNAL(clicked()), SLOT(sl_loadSequenceButton()));

    if(hideLoadSequences){
        loadSequenceButton->hide();
    }
}

void DotPlotDialog::accept() {
    int xIdx = xAxisCombo->currentIndex();
    int yIdx = yAxisCombo->currentIndex();
    SAFE_POINT(xIdx >= 0 && xIdx < sequences.length(), QString("DotPlotDialog: index is out of range: %1").arg(xIdx),);
    SAFE_POINT(yIdx >= 0 && yIdx < sequences.length(), QString("DotPlotDialog: index is out of range: %1").arg(yIdx),);
        
    U2SequenceObject* objX = sequences[xIdx];
    U2SequenceObject* objY = sequences[yIdx];

    if (!isObjectInADV(objX)) {
        adv->addObject(objX);
    }

    if (!isObjectInADV(objY)) {
        adv->addObject(objY);
    }

    xSeq = adv->getSequenceContext(objX);
    ySeq = adv->getSequenceContext(objY);

    QDialog::accept();
}

void DotPlotDialog::sl_minLenHeuristics() {
    identityBox->setValue(100);

    // formula used here: nVariations / lenVariations = wantedResCount (==1000)
    // where nVariations == area size
    // lenVariations = 4^len where len is result
    // so we have len = ln(nVariations/wantedResCount)/ln(4)

    int xIdx = xAxisCombo->currentIndex();
    int yIdx = yAxisCombo->currentIndex();
    
    U2SequenceObject *objX = sequences.at(xIdx);
    U2SequenceObject *objY = sequences.at(yIdx);

    qint64 xSeqLen = objX->getSequenceLength();
    qint64 ySeqLen = objY->getSequenceLength();

    double nVariations = xSeqLen*ySeqLen;
    double resCount = 1000;
    double len = log(nVariations / resCount) / log(double(4));

    minLenBox->setValue((int)len);
}

void DotPlotDialog::sl_hundredPercent() {
    identityBox->setValue(100);
}

int DotPlotDialog::getMismatches() const {
    return (100-identityBox->value()) * minLenBox->value()/ 100;
}

// which algorithm
RFAlgorithm DotPlotDialog::getAlgo() const {
    if (algoCheck->isChecked()) {
        int index = algoCombo->currentIndex();
        return RFAlgorithm(algoCombo->itemData(index).toInt());
    }
    return RFAlgorithm_Auto;
}

int DotPlotDialog::getMinLen() const {
    return minLenBox->value();
}

bool DotPlotDialog::isDirect() const {
    return directCheckBox->isChecked();
}

bool DotPlotDialog::isInverted() const {

    return invertedCheckBox->isChecked();
}

void DotPlotDialog::sl_directInvertedCheckBox() {

    startButton->setEnabled(directCheckBox->isChecked() || invertedCheckBox->isChecked());
}

static const QString COLOR_STYLE("QPushButton { background-color: %1 }");

void DotPlotDialog::sl_directColorButton() {
    QColorDialog d(directColor, this);

    if (d.exec()) {
        directColor = d.selectedColor();
        directCheckBox->setChecked(true);
    }

    updateColors();
}

void DotPlotDialog::sl_invertedColorButton() {
    QColorDialog d(invertedColor, this);

    if (d.exec()) {
        invertedColor = d.selectedColor();
        invertedCheckBox->setChecked(true);
    }

    updateColors();
}

void DotPlotDialog::sl_directDefaultColorButton() {
    directColor = QColor();
    directCheckBox->setChecked(true);
    updateColors();
}

void DotPlotDialog::sl_invertedDefaultColorButton() {
    invertedColor = QColor();
    invertedCheckBox->setChecked(true);
    updateColors();
}

void DotPlotDialog::sl_loadSequenceButton(){
    QString filter = DialogUtils::prepareDocumentsFileFilterByObjType(GObjectTypes::SEQUENCE, true);
    LastUsedDirHelper lod("DotPlot file");
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Open file"), lod.dir, filter);
    if(!lod.url.isEmpty()){
        Task *tasks = new Task("Adding document to the project", TaskFlag_NoRun);

        if (!AppContext::getProject()) {
            tasks->addSubTask( AppContext::getProjectLoader()->createNewProjectTask() );
        }

        //DotPlotLoadDocumentsTask *t = new DotPlotLoadDocumentsTask(lod.url, -1, NULL, -1, false);
        //tasks->addSubTask(t);
        QVariantMap hints;
        hints[ProjectLoaderHint_LoadWithoutView] = true;
        hints[ProjectLoaderHint_LoadUnloadedDocument] = true;
        openSequenceTask = AppContext::getProjectLoader()->openWithProjectTask(lod.url, hints);
        if(openSequenceTask == NULL){
            return;
        }
        curURL = lod.url;
        tasks->addSubTask(openSequenceTask);

        connect( AppContext::getTaskScheduler(), SIGNAL( si_stateChanged(Task*) ), SLOT( sl_loadTaskStateChanged(Task*) ) );

        AppContext::getTaskScheduler()->registerTopLevelTask(tasks);
    }

}

void DotPlotDialog::sl_loadTaskStateChanged(Task* t){
    DotPlotLoadDocumentsTask *loadTask = qobject_cast<DotPlotLoadDocumentsTask*>(t);
    if (!loadTask || !loadTask->isFinished()) {
            if(t->isFinished()){
                if(curURL == ""){
                    return;
                }
                GUrl URL(curURL);
                Project *project = AppContext::getProject();
                Q_ASSERT(project);
                Document *doc = project->findDocumentByURL(URL);
                if (!doc || !doc->isLoaded()) {
                    return;
                }
                QList<GObject*> docObjects  = doc->getObjects();
                foreach (GObject* obj, docObjects) {
                    U2SequenceObject* seqObj = qobject_cast<U2SequenceObject*>(obj);
                    if (seqObj != NULL){
                        QString name = seqObj->getGObjectName();
                        xAxisCombo->addItem(name);
                        yAxisCombo->addItem(name);
                        sequences << seqObj;
                    }
                }
                curURL = "";
            }
            return;
    }
    

    if (loadTask->getStateInfo().hasError()) {
        DotPlotDialogs::filesOpenError();
        return;
    }

    QList <Document *> docs = loadTask->getDocuments();
    foreach (Document* doc, docs) {
        QList<GObject*> docObjects  = doc->getObjects();
        foreach (GObject* obj, docObjects) {
            U2SequenceObject* seqObj = qobject_cast<U2SequenceObject*>(obj);
            if (seqObj != NULL){
                QString name = seqObj->getGObjectName();
                xAxisCombo->addItem(name);
                yAxisCombo->addItem(name);
                sequences << seqObj;
            }
        }
    }
}

void DotPlotDialog::updateColors() {
    directColorButton->setStyleSheet(COLOR_STYLE.arg(directColor.name()));
    invertedColorButton->setStyleSheet(COLOR_STYLE.arg(invertedColor.name()));
}

bool DotPlotDialog::isObjectInADV(GObject* obj){
    SAFE_POINT(obj != NULL, "Object is NULL in DotPlotDialog::isObjectInADV(GObject* obj)", false);

    QList<ADVSequenceObjectContext*> currentContexts = adv->getSequenceContexts();

    bool result = false;
    foreach(ADVSequenceObjectContext* objContext, currentContexts){
        if (objContext->getSequenceGObject()->getGObjectName() == obj->getGObjectName()){
            result = true;
        }
    }

    return result;
}

GObject* DotPlotDialog::getGObjectByName(const QString& gObjectName){
    QList<GObject*> allSequences  = GObjectUtils::findAllObjects(UOF_LoadedOnly, GObjectTypes::SEQUENCE);  
    GObject* obj = NULL;
    foreach (GObject* s, allSequences) {
        if (gObjectName == s->getGObjectName()) {
            obj = s; 
        }
    }
    return obj;
}

}//namespace
