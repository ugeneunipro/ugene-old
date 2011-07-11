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

#ifndef _U2_BAM_DBI_PLUGIN_H_
#define _U2_BAM_DBI_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/U2Dbi.h>
#include <U2Core/Log.h>
#include <U2Core/DocumentImport.h>
#include <U2Core/LoadDocumentTask.h>

namespace U2 {
namespace BAM {

class BAMDbiPlugin : public Plugin {
    Q_OBJECT
public:
    BAMDbiPlugin();
private slots:
    void sl_converter();
    void sl_infoLoaded(Task*);
    void sl_addDbFileToProject(Task*);
    
};

class BAMImporter : public DocumentImporter {
    Q_OBJECT
public:
    BAMImporter();

    virtual RawDataCheckResult checkRawData(const QByteArray& rawData, const GUrl& url);

    virtual DocumentProviderTask* createImportTask(const FormatDetectionResult& res, bool showWizard);

};

class LoadInfoTask;
class ConvertToSQLiteTask;

class BAMImporterTask : public DocumentProviderTask {
public:
    BAMImporterTask(const GUrl& url, bool useGui, bool sam);
    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    LoadInfoTask*           loadInfoTask;
    ConvertToSQLiteTask*    convertTask;
    LoadDocumentTask*       loadDocTask;
    bool                    useGui;
    bool                    sam;
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_DBI_PLUGIN_H_
