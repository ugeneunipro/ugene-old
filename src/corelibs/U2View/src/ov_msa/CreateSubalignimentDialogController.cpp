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

#include "CreateSubalignimentDialogController.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/TmpDirChecker.h>

#include <U2Formats/GenbankLocationParser.h>

#include <U2Algorithm/CreateSubalignmentTask.h>

#include <U2Gui/OpenViewTask.h>
#include <U2Gui/LastUsedDirHelper.h>


#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

#include <U2Core/BaseDocumentFormats.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/SaveDocumentGroupController.h>
#include <U2Gui/HelpButton.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/DocumentUtils.h>


namespace U2{

CreateSubalignimentDialogController::CreateSubalignimentDialogController(MAlignmentObject *_mobj, const QRect& selection, QWidget *p)
: QDialog(p), mobj(_mobj), saveContoller(NULL){
    setupUi(this);
    new HelpButton(this, buttonBox, "4227464");
    buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Extract"));
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

    connect(browseButton, SIGNAL(clicked()), SLOT(sl_browseButtonClicked()));
    connect(allButton, SIGNAL(clicked()), SLOT(sl_allButtonClicked()));
    connect(noneButton, SIGNAL(clicked()), SLOT(sl_noneButtonClicked()));
    connect(invertButton, SIGNAL(clicked()), SLOT(sl_invertButtonClicked()));

    int rowNumber = mobj->getNumRows();
    int alignLength = mobj->getLength();

    sequencesTableWidget->clearContents();
    sequencesTableWidget->setRowCount(rowNumber);
    sequencesTableWidget->setColumnCount(1);
    sequencesTableWidget->verticalHeader()->setHidden( true );
    sequencesTableWidget->horizontalHeader()->setHidden( true );
    sequencesTableWidget->setShowGrid(false);
    sequencesTableWidget->horizontalHeader()->setResizeMode( 0, QHeaderView::Stretch );

    SaveDocumentGroupControllerConfig conf;
    conf.dfc.addFlagToExclude(DocumentFormatFlag_SingleObjectFormat);
    conf.dfc.addFlagToSupport(DocumentFormatFlag_SupportWriting);
    conf.dfc.supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
    conf.fileDialogButton = browseButton;
    conf.formatCombo = formatCombo;
    conf.fileNameEdit = filepathEdit;
    conf.parentWidget = this;
    conf.defaultFormatId = BaseDocumentFormats::CLUSTAL_ALN;
    QString fileName = GUrlUtils::rollFileName(_mobj->getDocument()->getURLString(), "_subalign", DocumentUtils::getNewDocFileNameExcludesHint());
    conf.defaultFileName = fileName;

    saveContoller = new SaveDocumentGroupController(conf, this);

    int startSeq = -1;
    int endSeq = -1;
    int startPos = -1;
    int endPos = -1;
    if (selection.isNull()) {
        startPos = 1;
        endPos = alignLength;
        startSeq = 0;
        endSeq = rowNumber -1;
     } else {
        startSeq = selection.y();
        endSeq = selection.y() + selection.height() - 1;
        startPos = selection.x() + 1;
        endPos = selection.x() + selection.width();
    }
    
    startPosBox->setMaximum(alignLength);
    endPosBox->setMaximum(alignLength);
    
    startPosBox->setValue(startPos);
    endPosBox->setValue(endPos);
    
    for (int i=0; i<rowNumber; i++) {
        QCheckBox *cb = new QCheckBox(mobj->getMAlignment().getRow(i).getName(), this);
        cb->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
        if ( (i >= startSeq) && (i <= endSeq)) {
            cb->setChecked(true);
        }
        sequencesTableWidget->setCellWidget(i, 0, cb);
        sequencesTableWidget->setRowHeight(i, 15);
    }

}

QString CreateSubalignimentDialogController::getSavePath(){ 
    if(NULL == saveContoller) {
        return QString();
    }
    return saveContoller->getSaveFileName(); 
};
DocumentFormatId CreateSubalignimentDialogController::getFormatId() {
    if(NULL == saveContoller) {
        return DocumentFormatId();
    }
    return saveContoller->getFormatIdToSave();
}

void CreateSubalignimentDialogController::sl_allButtonClicked(){
    for (int i=0; i<sequencesTableWidget->rowCount(); i++) {
        QCheckBox *cb = qobject_cast<QCheckBox*>(sequencesTableWidget->cellWidget(i, 0));
        cb->setChecked(true);
    }
}

void CreateSubalignimentDialogController::sl_invertButtonClicked(){
    for (int i=0; i<sequencesTableWidget->rowCount(); i++) {
        QCheckBox *cb = qobject_cast<QCheckBox*>(sequencesTableWidget->cellWidget(i, 0));
        cb->setChecked(!cb->isChecked());
    }
}

void CreateSubalignimentDialogController::sl_noneButtonClicked(){
    for (int i=0; i<sequencesTableWidget->rowCount(); i++) {
        QCheckBox *cb = qobject_cast<QCheckBox*>(sequencesTableWidget->cellWidget(i, 0));
        cb->setChecked(false);
    }
}

void CreateSubalignimentDialogController::accept(){
    QFileInfo fi(filepathEdit->text());
    QDir dirToSave(fi.dir());
    if (!dirToSave.exists()){
        QMessageBox::critical(this, this->windowTitle(), tr("Directory to save is not exists"));
        return;
    }
    if (!TmpDirChecker::checkWritePermissions(dirToSave.absolutePath())) {
        QMessageBox::critical(this, this->windowTitle(), tr("No write permission to '%1' directory").arg(dirToSave.absolutePath()));
        return;
    }
    if(filepathEdit->text().isEmpty()){
        QMessageBox::critical(this, this->windowTitle(), tr("No path specified"));
        return;
    }
    if(fi.baseName().isEmpty()){
        QMessageBox::critical(this, this->windowTitle(), tr("Filename to save is empty"));
        return;
    }
    if (!fi.permissions().testFlag(QFile::WriteUser)) {
        QMessageBox::critical(this, this->windowTitle(), tr("No write permission to '%1' file").arg(fi.fileName()));
        return;
    }
    
    // '-1' because in memory positions start from 0 not 1
    int start = startPosBox->value() - 1;
    int end = endPosBox->value() - 1;
    int seqLen = mobj->getLength();
    
    if( start > end ) {
        QMessageBox::critical(this, windowTitle(), tr("Start position must be less than end position!"));
        return;
    }
    
    U2Region region(start, end - start + 1), sequence(0, seqLen);
    if(!sequence.contains(region)){
        QMessageBox::critical(this, this->windowTitle(), tr("Entered region not contained in current sequence"));
        return;
    }

    selectSeqNames();

    if(selectedNames.size() == 0){
        QMessageBox::critical(this, this->windowTitle(), tr("You must select at least one sequence"));
        return;
    }

    window = region;

    this->close();
    QDialog::accept();
}

void CreateSubalignimentDialogController::selectSeqNames(){
    QStringList names;
    for (int i=0; i<sequencesTableWidget->rowCount(); i++) {
        QCheckBox *cb = qobject_cast<QCheckBox*>(sequencesTableWidget->cellWidget(i, 0));
        if(cb->isChecked()){
            names.append(cb->text());
        }
    }
    selectedNames = names;
}


CreateSubalignmentAndOpenViewTask::CreateSubalignmentAndOpenViewTask( MAlignmentObject* maObj, const CreateSubalignmentSettings& settings )
:Task(tr("Create sub-alignment and open view: %1").arg(maObj->getDocument()->getName()), TaskFlags_NR_FOSCOE)
{
    csTask = new CreateSubalignmentTask(maObj, settings);
    addSubTask(csTask);
    setMaxParallelSubtasks(1);
}

QList<Task*> CreateSubalignmentAndOpenViewTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    CHECK_OP(stateInfo, res);
    
    if ((subTask == csTask) && csTask->getSettings().addToProject) {
        Document* doc = csTask->takeDocument();
        assert(doc != NULL);
        res.append(new AddDocumentAndOpenViewTask(doc));
    }

    return res;

}


};
