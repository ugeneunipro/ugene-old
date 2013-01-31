/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "SecStructDialog.h"

#include <U2Core/U2Region.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/CreateAnnotationTask.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/PluginModel.h>

#include <U2Algorithm/SecStructPredictAlgRegistry.h>
#include <U2Algorithm/SecStructPredictTask.h>

#include <U2Gui/CreateAnnotationDialog.h>
#include <U2Gui/CreateAnnotationWidgetController.h>

#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/LicenseDialog.h>

#include <QtCore/QMutableListIterator>
#include <QtGui/QHeaderView>

namespace U2 {

SecStructDialog::SecStructDialog( ADVSequenceObjectContext* _ctx, QWidget *p ) : QDialog(p), rangeStart(0), rangeEnd(0), ctx(_ctx), task(NULL)
{
    setupUi(this);
    sspr = AppContext::getSecStructPredictAlgRegistry();
    algorithmComboBox->addItems(sspr->getAlgNameList());
    saveAnnotationButton->setDisabled(true);
    
    U2Region initialSelection = ctx->getSequenceSelection()->isEmpty() ? U2Region() : ctx->getSequenceSelection()->getSelectedRegions().first();

    int seqLen = ctx->getSequenceLength();

    rangeStartSpinBox->setMinimum(1);
    rangeStartSpinBox->setMaximum(seqLen);

    rangeEndSpinBox->setMinimum(1);
    rangeEndSpinBox->setMaximum(seqLen);

    rangeStartSpinBox->setValue(initialSelection.isEmpty() ? 1 : initialSelection.startPos + 1);
    rangeEndSpinBox->setValue(initialSelection.isEmpty() ? seqLen : initialSelection.endPos());

    resultsTable->setColumnCount(2);
    QStringList headerNames;
    headerNames.append(tr("Region"));
    headerNames.append(tr("Structure Type"));
    resultsTable->setHorizontalHeaderLabels(headerNames);
    resultsTable->horizontalHeader()->setStretchLastSection(true);
    
    connect(AppContext::getTaskScheduler(), SIGNAL(si_stateChanged(Task*)), SLOT(sl_onTaskFinished(Task*)));
    connectGUI();

}

void SecStructDialog::connectGUI()
{
    connect(rangeStartSpinBox, SIGNAL(valueChanged(int)), this, SLOT(sl_spinRangeStartChanged(int)));
    connect(rangeEndSpinBox, SIGNAL(valueChanged(int)), this, SLOT(sl_spinRangeEndChanged(int)));
    connect(startButton, SIGNAL(clicked()), this, SLOT(sl_onStartPredictionClicked()));
    connect(saveAnnotationButton, SIGNAL(clicked()), this, SLOT(sl_onSaveAnnotations()));

    
}

void SecStructDialog::sl_spinRangeStartChanged( int val )
{
    if (val > rangeEndSpinBox->value()) {
        rangeEndSpinBox->setValue(val);
    }
    
}

void SecStructDialog::sl_spinRangeEndChanged( int val )
{
    if (val < rangeStartSpinBox->value()) {
        rangeStartSpinBox->setValue(val);
    }
}

void SecStructDialog::updateState()
{
    bool haveActiveTask = task!=NULL;
    bool haveResults = !results.isEmpty();

    algorithmComboBox->setEnabled(!haveActiveTask);
    startButton->setEnabled(!haveActiveTask);
    cancelButton->setEnabled(!haveActiveTask);
    saveAnnotationButton->setEnabled(haveResults);
    totalPredictedStatus->setText( QString("%1").arg(results.size()));
    showResults();

}

void SecStructDialog::sl_onStartPredictionClicked() {
    SAFE_POINT(task == NULL, "Found pending prediction task!", );

    SecStructPredictTaskFactory* factory = sspr->getAlgorithm(algorithmComboBox->currentText());

    //Check license
    QString algorithm=algorithmComboBox->currentText();
    QList<Plugin*> plugins=AppContext::getPluginSupport()->getPlugins();
    foreach (Plugin* plugin, plugins){
        if(plugin->getName() == algorithm){
            if(!plugin->isFree() && !plugin->isLicenseAccepted()){
                LicenseDialog licenseDialog(plugin);
                int ret = licenseDialog.exec();
                if(ret != QDialog::Accepted){
                    return;
                }
            }
            break;
        }
    }

    //prepare target sequence
    rangeStart = rangeStartSpinBox->value();
    rangeEnd = rangeEndSpinBox->value();
    
    SAFE_POINT(rangeStart <= rangeEnd, "Illegal region!", );
    SAFE_POINT(rangeStart >= 0 && rangeEnd <= ctx->getSequenceLength(), "Illegal region!", );
    
    U2Region r(rangeStart, rangeEnd - rangeStart);
    QByteArray seqPart = ctx->getSequenceData(r);
    task = factory->createTaskInstance(seqPart);
    AppContext::getTaskScheduler()->registerTopLevelTask(task);
    results.clear();
    
    updateState();
}

void SecStructDialog::sl_onTaskFinished(Task* t) {
    if (t != task || t->getState()!= Task::State_Finished) {
        return;
    }
    results = task->getResults();

    //shifting results according to startPos
    for(QMutableListIterator<SharedAnnotationData> it_ad(results); it_ad.hasNext(); ) {
        AnnotationData * ad = it_ad.next().data();
        U2Region::shift(rangeStart, ad->location->regions);
    }
    task = NULL;
    rangeStart = 0;
    rangeEnd = 0;
    updateState();

}

void SecStructDialog::showResults()
{
    int rowIndex = 0;
    resultsTable->setRowCount(results.size());
    foreach(SharedAnnotationData data, results) {
        U2Region annRegion = data->getRegions().first();
        QTableWidgetItem *locItem = new QTableWidgetItem( QString("[%1..%2]").arg(annRegion.startPos).
            arg(annRegion.endPos()) );
        resultsTable->setItem(rowIndex, 0, locItem);
        QTableWidgetItem* nameItem = new QTableWidgetItem( QString(data->name));
        resultsTable->setItem(rowIndex, 1, nameItem);
        ++rowIndex;
    }
    

}

#define SEC_STRUCT_ANNOTATION_GROUP_NAME "predicted"

void SecStructDialog::sl_onSaveAnnotations()
{
    CreateAnnotationModel m;
    m.sequenceObjectRef = ctx->getSequenceObject();
    m.hideLocation = true;
    m.hideAnnotationName = true;
    m.data->name = SEC_STRUCT_ANNOTATION_GROUP_NAME;
    m.sequenceLen = ctx->getSequenceObject()->getSequenceLength();
    CreateAnnotationDialog d(this, m);
    int rc = d.exec();
    if (rc != QDialog::Accepted) {
        return;
    }
    CreateAnnotationsTask* t = new CreateAnnotationsTask(m.getAnnotationObject(), m.groupName, results);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);

    QDialog::accept();

    
}
} // namespace


