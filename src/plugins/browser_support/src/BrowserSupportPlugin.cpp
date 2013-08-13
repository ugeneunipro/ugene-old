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

#include "BrowserSupportPlugin.h"
#include <U2Core/AppContext.h>
#include <U2Gui/OpenViewTask.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/Counter.h>


#include <QtGui/QClipboard>
#include <QtGui/QApplication>



#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/Log.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/NetworkConfiguration.h>
#include <U2Core/Counter.h>
#include <U2Core/DBXRefRegistry.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/CopyDataTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/U2SafePoints.h>

#include <U2Core/StringAdapter.h>

#include <U2Core/LoadRemoteDocumentTask.h>

#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QUrl>

#include <QtNetwork/QNetworkProxy>

#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/GUrlUtils.h>


#include <iostream>

using namespace std;

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    if (AppContext::getMainWindow()) {
        BrowserSupportPlugin* plug = new BrowserSupportPlugin();
        return plug;
    }
    return NULL;
}

BrowserSupportPlugin::BrowserSupportPlugin() : Plugin(tr("BrowserSupport"), tr("Browser support")) {
    connect(QApplication::clipboard(), SIGNAL(changed(QClipboard::Mode)), SLOT(sl_clipboardCheck()));
}

static bool clipboardRead = false;
static QString prevClipboard;

QString BrowserSupportPlugin::saveToTmp(QString &data) {

    QString tmpDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath();

    U2OpStatus2Log os;
    QString fileName = GUrlUtils::prepareTmpFileLocation(tmpDirPath, "clipboard_data", "clipboard", os);
    int i = 100;

    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream out(&file);
    out << data;

    return fileName;
}

void BrowserSupportPlugin::sl_clipboardCheck() {

    QClipboard *clipboard = QApplication::clipboard();
    QString clipboardText = clipboard->text();

    if (prevClipboard != clipboardText) {

        int newLine = clipboardText.indexOf("\n");
        QString noOpen  = QString(clipboardText.data() + newLine).trimmed();
        if (newLine < 0) {
            prevClipboard = clipboardText;
            return;
        }

        if (clipboardText.startsWith("open/selection;")) {
            GCOUNTER( cvar, tvar, "BrowserSupport:open/selection" );
            QString file = saveToTmp(noOpen);

            QList<GUrl> urls; urls << GUrl(file, GUrl_File);
            QVariantMap hints;
//            hints[ProjectLoaderHint_ForceFormatOptions] = true;
            Task* openTask = AppContext::getProjectLoader()->openWithProjectTask(urls, hints);
            if (openTask != NULL) {
                AppContext::getTaskScheduler()->registerTopLevelTask(openTask);	
                QApplication::clipboard()->setText("");
            }
        }

        QString fullPath = LoadRemoteDocumentTask::getDefaultDownloadDirectory();
        LoadRemoteDocumentTask::prepareDownloadDirectory(fullPath);
        fullPath = QDir::toNativeSeparators(fullPath);

        if (clipboardText.startsWith("open/Ensembl;")) {
            GCOUNTER( cvar, tvar, "BrowserSupport:open/Ensembl" );
            int idLength = noOpen.indexOf("\n");
            QString ensId = noOpen.left(idLength);
            ensId = ensId.simplified();

            Task* task = new LoadRemoteDocumentAndOpenViewTask(ensId, "ENSEMBL", fullPath);
            AppContext::getTaskScheduler()->registerTopLevelTask(task);
            QApplication::clipboard()->setText("");
        }

        if (clipboardText.startsWith("open/PDB;")) {
            GCOUNTER( cvar, tvar, "BrowserSupport:open/PDB" );
            int idLength = noOpen.indexOf("\n");
            QString ensId = noOpen.left(idLength);
            ensId = ensId.simplified();

            Task* task = new LoadRemoteDocumentAndOpenViewTask(ensId, "PDB", fullPath);
            AppContext::getTaskScheduler()->registerTopLevelTask(task);
            QApplication::clipboard()->setText("");
        }
    }
    prevClipboard = clipboardText;
}

} //namespace
