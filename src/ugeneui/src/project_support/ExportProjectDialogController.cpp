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

#include "ExportProjectDialogController.h"

#include <U2Core/GUrlUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/LastUsedDirHelper.h>

#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>

namespace U2{

static QString fixProjectFile(const QString& name) {
    QString result = name;
    if (result.isEmpty()) {
        result =  "project" + PROJECTFILE_EXT;
    } else if (!result.endsWith(PROJECTFILE_EXT)) {
        result+=PROJECTFILE_EXT;
    }
    return result;
}

ExportProjectDialogController::ExportProjectDialogController(QWidget *p, const QString& defaultProjectFileName) 
: QDialog(p) 
{
	setupUi(this);
    setModal(true);
    projectFile = fixProjectFile(defaultProjectFileName);
    projectFileEdit->setText(projectFile);
    Project* proj = AppContext::getProject();
    if (proj == NULL || !proj->isItemModified()) {
        warningLabel->setVisible(false);
    }
	connect(browseButton, SIGNAL(clicked()), this, SLOT(sl_onBrowseButton()));
}

void ExportProjectDialogController::accept(){
    QString dirPath = exportFolderEdit->text();
    projectFile = fixProjectFile(projectFileEdit->text());
    
    U2OpStatus2Log os;
    exportDir = GUrlUtils::prepareDirLocation(dirPath, os);
    if (exportDir.isEmpty()) {
        assert(os.hasError());
        QMessageBox::critical(this, this->windowTitle(), os.getError());
        return;
    }
	QDialog::accept();
}

void ExportProjectDialogController::sl_onBrowseButton(){
	LastUsedDirHelper h;
	QString folder = QFileDialog::getExistingDirectory(this, tr("Choose Directory"), h.dir);
    if (folder.isEmpty()) {
        return;
    }
	exportFolderEdit->setText(folder);
}

}
