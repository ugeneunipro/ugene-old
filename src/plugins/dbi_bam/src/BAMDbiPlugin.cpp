#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Gui/MainWindow.h>
#include "BAMFormat.h"
#include "Dbi.h"
#include "Header.h"
#include "Reader.h"
#include "IOException.h"
#include "ConvertToSQLiteDialog.h"
#include "SelectReferencesDialog.h"
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
    AppContext::getDocumentFormatRegistry()->registerFormat(new BAMFormat());
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
        ConvertToSQLiteDialog convertDialog;
        if(QDialog::Accepted == convertDialog.exec()) {
            ConvertToSQLiteTask *task = new ConvertToSQLiteTask(convertDialog.getSourceUrl(), convertDialog.getDestinationUrl());
            AppContext::getTaskScheduler()->registerTopLevelTask(task);
        }
    } catch(const Exception &e) {
        QMessageBox::critical(NULL, tr("Error"), e.getMessage());
    }
}

} // namespace BAM
} // namespace U2
