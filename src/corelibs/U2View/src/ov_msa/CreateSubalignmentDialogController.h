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

#ifndef _U2_CREATE_SUBALIGNIMENT_DIALOG_CONTROLLER_H_
#define _U2_CREATE_SUBALIGNIMENT_DIALOG_CONTROLLER_H_

#include "ui/ui_CreateSubalignmentDialog.h"

#include <U2Core/global.h>
#include <U2Core/Task.h>
#include <U2Core/MAlignmentObject.h>

namespace U2{

class SaveDocumentGroupController;

class U2VIEW_EXPORT CreateSubalignmentDialogController : public QDialog, Ui_CreateSubalignmentDialog {
    Q_OBJECT
public:
    CreateSubalignmentDialogController( MAlignmentObject *_mobj, const QRect& selection, QWidget *p = NULL);

    void accept();

    bool getAddToProjFlag(){return addToProjBox->isChecked();};
    QString getSavePath();
    DocumentFormatId getFormatId();
    U2Region getRegion(){return window;};
    QStringList getSelectedSeqNames(){return selectedNames;};

private slots:
    void sl_allButtonClicked();
    void sl_invertButtonClicked();
    void sl_noneButtonClicked();

private:
    void selectSeqNames();

    MAlignmentObject *mobj;
    U2Region window;
    QStringList selectedNames;
    SaveDocumentGroupController* saveContoller;
};


class CreateSubalignmentTask;
class CreateSubalignmentSettings;

class U2VIEW_EXPORT CreateSubalignmentAndOpenViewTask : public Task {
    Q_OBJECT
public:
    CreateSubalignmentAndOpenViewTask(MAlignmentObject* mobj, const CreateSubalignmentSettings& settings);
    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    CreateSubalignmentTask* csTask;

};




}

#endif
