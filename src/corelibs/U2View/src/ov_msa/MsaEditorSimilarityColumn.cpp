/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include "MsaEditorSimilarityColumn.h"
#include "MSAEditor.h"
#include "MSAEditorSequenceArea.h"

#include <U2Core/AppContext.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MAlignment.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Algorithm/MSADistanceAlgorithmRegistry.h>
#include <U2Algorithm/MSADistanceAlgorithm.h>

namespace U2
{

const QString MsaEditorAlignmentDependentWidget::DataIsOutdatedMessage(QString("<FONT COLOR=#FF0000>%1</FONT>").arg(QObject::tr("Data is outdated")));
const QString MsaEditorAlignmentDependentWidget::DataIsValidMessage(QString("<FONT COLOR=#00FF00>%1</FONT>").arg(QObject::tr("Data is valid")));
const QString MsaEditorAlignmentDependentWidget::DataIsBeingUpdatedMessage(QString("<FONT COLOR=#0000FF>%1</FONT>").arg(QObject::tr("Data is being updated")));

MsaEditorSimilarityColumn::MsaEditorSimilarityColumn(MSAEditorUI* ui, QScrollBar* nhBar, const SimilarityStatisticsSettings* _settings)
    : MSAEditorNameList(ui, nhBar),
      matrix(NULL),
      autoUpdate(true)
{
    newSettings = curSettings = *_settings;
    updateDistanceMatrix();
    setObjectName("msa_editor_similarity_column");
}

MsaEditorSimilarityColumn::~MsaEditorSimilarityColumn() {
    CHECK(NULL != matrix, );
    delete matrix;
}

QString MsaEditorSimilarityColumn::getTextForRow( int s ) {
    if (NULL == matrix || state == DataIsBeingUpdated) {
        return tr("-");
    }

    const MAlignment& ma = editor->getMSAObject()->getMAlignment();
    const qint64 referenceRowId = editor->getReferenceRowId();
    if(MAlignmentRow::invalidRowId() == referenceRowId) {
        return tr("-");
    }

    U2OpStatusImpl os;
    const int refSequenceIndex = ma.getRowIndexByRowId(referenceRowId, os);
    CHECK_OP(os, QString());

    int sim = matrix->getSimilarity(refSequenceIndex, s);
    CHECK(-1 != sim, tr("-"));
    const QString units = matrix->areUsePercents() ? "%" : "";
    return QString("%1").arg(sim) + units;
}

QString MsaEditorSimilarityColumn::getSeqName(int s) {
    const MAlignment& ma = editor->getMSAObject()->getMAlignment();

    return ma.getRowNames().at(s);
}

void MsaEditorSimilarityColumn::updateScrollBar() {
    // do nothing
}

void MsaEditorSimilarityColumn::setSettings(const UpdatedWidgetSettings* _settings) {
    const SimilarityStatisticsSettings* set= static_cast<const SimilarityStatisticsSettings*>(_settings);
    CHECK(NULL != set,);
    autoUpdate = set->autoUpdate;
    state = DataIsValid;
    if(curSettings.algoName != set->algoName) {
        state = DataIsOutdated;
    }
    if(curSettings.excludeGaps != set->excludeGaps) {
        state = DataIsOutdated;
    }
    if(curSettings.usePercents != set->usePercents) {
        if(NULL != matrix) {
            matrix->showSimilarityInPercents(set->usePercents);
            updateContent();
        }
        curSettings.usePercents = set->usePercents;
    }
    newSettings = *set;
    if(autoUpdate && DataIsOutdated == state) {
        state = DataIsBeingUpdated;
        emit si_dataStateChanged(state);
        updateDistanceMatrix();
    }
    emit si_dataStateChanged(state);
}

QString MsaEditorSimilarityColumn::getHeaderText() const {
    return curSettings.usePercents ? "%" : tr("score");
}

void MsaEditorSimilarityColumn::updateDistanceMatrix() {
    createDistanceMatrixTaskRunner.cancel();

    CreateDistanceMatrixTask* createDistanceMatrixTask = new CreateDistanceMatrixTask(newSettings);
    connect(new TaskSignalMapper(createDistanceMatrixTask), SIGNAL(si_taskFinished(Task*)), this, SLOT(sl_createMatrixTaskFinished(Task*)));

    state = DataIsBeingUpdated;
    createDistanceMatrixTaskRunner.run( createDistanceMatrixTask );
}

void MsaEditorSimilarityColumn::onAlignmentChanged(const MAlignment&, const MAlignmentModInfo&) {
    if(autoUpdate) {
        state = DataIsBeingUpdated;
        updateDistanceMatrix();
    }
    else {
        state = DataIsOutdated;
    }
    emit si_dataStateChanged(state);
}

void MsaEditorSimilarityColumn::sl_createMatrixTaskFinished(Task* t) {
    CreateDistanceMatrixTask* task = qobject_cast<CreateDistanceMatrixTask*> (t);
    if(NULL != task && !task->hasError() && !task->isCanceled()) {
        if(NULL != matrix) {
            delete matrix;
        }
        matrix = task->getResult();
        if(NULL != matrix) {
            matrix->showSimilarityInPercents(newSettings.usePercents);
        }
    }
    updateContent();
    state = DataIsValid;
    curSettings = newSettings;
    emit si_dataStateChanged(state);
}

CreateDistanceMatrixTask::CreateDistanceMatrixTask(const SimilarityStatisticsSettings& _s)
    : BackgroundTask<MSADistanceMatrix*>(tr("Generate distance matrix"), TaskFlags_NR_FOSE_COSC),
      s(_s),
      resMatrix(NULL) {
    SAFE_POINT(NULL != s.ma, QString("Incorrect MAlignment in MsaEditorSimilarityColumnTask ctor!"), );
    SAFE_POINT(NULL != s.ui, QString("Incorrect MSAEditorUI in MsaEditorSimilarityColumnTask ctor!"), );
    setVerboseLogMode(true);
}

void CreateDistanceMatrixTask::prepare() {
    MSADistanceAlgorithmFactory* factory = AppContext::getMSADistanceAlgorithmRegistry()->getAlgorithmFactory(s.algoName);
    CHECK(NULL != factory,);
    if(s.excludeGaps){
        factory->setFlag(DistanceAlgorithmFlag_ExcludeGaps);
    }else{
        factory->resetFlag(DistanceAlgorithmFlag_ExcludeGaps);
    }

    MSADistanceAlgorithm* algo = factory->createAlgorithm(s.ma->getMAlignment());
    //connect(s.ma, SIGNAL(si_rowsAdded()), algo, SLOT(sl_addEmptyDistanceRow()));
    CHECK(NULL != algo,);
    addSubTask(algo);
}

QList<Task*> CreateDistanceMatrixTask::onSubTaskFinished(Task* subTask){
    QList<Task*> res;
    if (isCanceled()) {
        return res;
    }
    MSADistanceAlgorithm* algo = qobject_cast<MSADistanceAlgorithm*>(subTask);
    MSADistanceMatrix *matrix = new MSADistanceMatrix(algo, s.usePercents && s.excludeGaps);
    if(NULL != algo) {
        if(algo->hasError()) {
            setError(algo->getError());
            return res;
        }
        resMatrix = matrix;
    }
    return res;
}
MsaEditorAlignmentDependentWidget::MsaEditorAlignmentDependentWidget(UpdatedWidgetInterface* _contentWidget)
: contentWidget(_contentWidget), automaticUpdating(true){
    SAFE_POINT(NULL != _contentWidget, QString("Argument is NULL in constructor MsaEditorAlignmentDependentWidget()"),);

    settings = &contentWidget->getSettings();
    connect(settings->ma, SIGNAL(si_alignmentChanged(const MAlignment&, const MAlignmentModInfo&)),
        this, SLOT(sl_onAlignmentChanged(const MAlignment&, const MAlignmentModInfo&)));
    connect(dynamic_cast<QObject*>(contentWidget), SIGNAL(si_dataStateChanged(DataState)),
        this, SLOT(sl_onDataStateChanged(DataState)));
    connect(settings->ui->getEditor(), SIGNAL(si_fontChanged(const QFont&)), SLOT(sl_onFontChanged(const QFont&)));

    createWidgetUI();

    setSettings(settings);
}
void MsaEditorAlignmentDependentWidget::createWidgetUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout();

    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);

    createHeaderWidget();

    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(contentWidget->getWidget());
    nameWidget.setText(contentWidget->getHeaderText());
    nameWidget.setObjectName("Distance column name");

    this->setLayout(mainLayout);
}
void MsaEditorAlignmentDependentWidget::createHeaderWidget() {
    QVBoxLayout* headerLayout = new QVBoxLayout();
    headerLayout->setMargin(0);
    headerLayout->setSpacing(0);

    nameWidget.setAlignment(Qt::AlignCenter);
    nameWidget.setFont(settings->ui->getEditor()->getFont());
    headerLayout->addWidget(&nameWidget);

    state = DataIsValid;
    headerWidget = new MSAWidget(settings->ui);
    headerWidget->setLayout(headerLayout);
}

void MsaEditorAlignmentDependentWidget::setSettings(const UpdatedWidgetSettings* _settings) {
    settings = _settings;
    automaticUpdating = settings->autoUpdate;
    contentWidget->setSettings(settings);
    nameWidget.setText(contentWidget->getHeaderText());
}

void MsaEditorAlignmentDependentWidget::sl_onAlignmentChanged(const MAlignment& maBefore, const MAlignmentModInfo& modInfo) {
    contentWidget->onAlignmentChanged(maBefore, modInfo);
}

void MsaEditorAlignmentDependentWidget::sl_onUpdateButonPressed() {
    contentWidget->updateWidget();
}

void MsaEditorAlignmentDependentWidget::sl_onDataStateChanged(DataState newState) {
    state = DataIsValid;
    switch(newState) {
        case DataIsValid:
            statusBar.setText(DataIsValidMessage);
            updateButton.setEnabled(false);
            break;
        case DataIsBeingUpdated:
            statusBar.setText(DataIsBeingUpdatedMessage);
            updateButton.setEnabled(false);
            break;
        case DataIsOutdated:
            statusBar.setText(DataIsOutdatedMessage);
            updateButton.setEnabled(true);
            break;
    }
}

void MsaEditorAlignmentDependentWidget::sl_onFontChanged(const QFont& font) {
    nameWidget.setFont(font);
}

} //namespace


