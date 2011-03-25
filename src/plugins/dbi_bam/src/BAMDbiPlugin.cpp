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

#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <U2Core/AppContext.h>
#include <U2Core/DbiDocumentFormat.h>
#include <U2Core/TaskSignalMapper.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Gui/OpenViewTask.h>
#include <U2Gui/MainWindow.h>
#include "Dbi.h"
#include "Exception.h"
#include "ConvertToSQLiteDialog.h"
#include "ConvertToSQLiteTask.h"
#include "BAMDbiPlugin.h"

namespace U2 {
namespace BAM {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    BAMDbiPlugin* plug = new BAMDbiPlugin();
    return plug;
}

BAMDbiPlugin::BAMDbiPlugin() : Plugin(tr("BAM format support"), tr("Interface for indexed read-only access to BAM files"))
{
    AppContext::getDocumentFormatRegistry()->registerFormat(new DbiDocumentFormat(DbiFactory::ID, "bam", tr("BAM File"), QStringList("bam")));
    AppContext::getDbiRegistry()->registerDbiFactory(new DbiFactory());

    {
        MainWindow *mainWindow = AppContext::getMainWindow();
        if(NULL != mainWindow) {
            QAction *converterAction = new QAction(tr("Import BAM File..."), this);
            connect(converterAction, SIGNAL(triggered()), SLOT(sl_converter()));
            mainWindow->getTopLevelMenu(MWMENU_TOOLS)->addAction(converterAction);
        }
    }
}

void BAMDbiPlugin::sl_converter() {
    try {
        if(!AppContext::getDbiRegistry()->getRegisteredDbiFactories().contains("SQLiteDbi")) {
            throw Exception(tr("SQLite DBI plugin is not loaded"));
        }
        ConvertToSQLiteDialog convertDialog(AppContext::getProject() != NULL);
        if(QDialog::Accepted == convertDialog.exec()) {
            ConvertToSQLiteTask *task = new ConvertToSQLiteTask(convertDialog.getSourceUrl(), convertDialog.getDestinationUrl());
            if(convertDialog.addToProject()) {
                connect(new TaskSignalMapper(task), SIGNAL(si_taskFinished(Task*)), SLOT(sl_addDbFileToProject(Task*)));
            }
            AppContext::getTaskScheduler()->registerTopLevelTask(task);
        }
    } catch(const Exception &e) {
        QMessageBox::critical(NULL, tr("Error"), e.getMessage());
    }
}

void BAMDbiPlugin::sl_addDbFileToProject(Task * task) {
    ConvertToSQLiteTask * convertToBAMTask = qobject_cast<ConvertToSQLiteTask*>(task);
    if(convertToBAMTask == NULL) {
        assert(false);
        return;
    }
    if(convertToBAMTask->hasErrors() || convertToBAMTask->isCanceled()) {
        return;
    }
    GUrl url = convertToBAMTask->getDestinationUrl();
    assert(!url.isEmpty());
    DocumentFormat * df = AppContext::getDocumentFormatRegistry()->getFormatById("usqlite");
    if(df == NULL) {
        assert(false);
        return;
    }
    IOAdapterFactory * iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(url.getURLString()));
    assert(iof != NULL);
    Document * doc = new Document(df, iof, url);
    AddDocumentTask * addTask = new AddDocumentTask(doc);
    LoadUnloadedDocumentAndOpenViewTask * openViewTask = new LoadUnloadedDocumentAndOpenViewTask(doc);
    openViewTask->addSubTask(addTask);
    openViewTask->setMaxParallelSubtasks(1);
    AppContext::getTaskScheduler()->registerTopLevelTask(openViewTask);
}

} // namespace BAM
} // namespace U2
