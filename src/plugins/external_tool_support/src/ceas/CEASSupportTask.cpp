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

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/TextObject.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include "CEASSupport.h"
#include "ExternalToolRunTask.h"

#include "CEASSupportTask.h"

namespace U2 {

/************************************************************************/
/* Settings */
/************************************************************************/
const QString CEASTaskSettings::PDF_FORMAT("PDF");
const QString CEASTaskSettings::PNG_FORMAT("PNG");

CEASTaskSettings::CEASTaskSettings() {

}
    
CEASTaskSettings::CEASTaskSettings(const CEASSettings &_ceas, const QList<SharedAnnotationData> &_bedData,
                                   const QString &_wigData)
: ceas(_ceas), bedData(_bedData), wigData(_wigData)
{

}

CEASSettings & CEASTaskSettings::getCeasSettings() {
    return ceas;
}

const CEASSettings & CEASTaskSettings::getCeasSettings() const {
    return ceas;
}

const QList<SharedAnnotationData> & CEASTaskSettings::getBedData() const {
    return bedData;
}

const QString & CEASTaskSettings::getWigData() const {
    return wigData;
}

/************************************************************************/
/* Task */
/************************************************************************/
const QString CEASSupportTask::BASE_DIR_NAME("ceas_report");

CEASSupportTask::CEASSupportTask(const CEASTaskSettings &_settings)
: Task("Running CEAS report task", TaskFlag_None),
settings(_settings), bedDoc(NULL), wigDoc(NULL),
bedTask(NULL), wigTask(NULL), etTask(NULL), activeSubtasks(0), logParser(NULL)
{

}

CEASSupportTask::~CEASSupportTask() {
    cleanup();
}

void CEASSupportTask::cleanup() {
    delete bedDoc; bedDoc = NULL;
    delete wigDoc; wigDoc = NULL;
    delete logParser; logParser = NULL;
}

void CEASSupportTask::prepare() {
    UserAppsSettings *appSettings = AppContext::getAppSettings()->getUserAppsSettings();
    workingDir = appSettings->createCurrentProcessTemporarySubDir(stateInfo, BASE_DIR_NAME);
    CHECK_OP(stateInfo, );

    createBedDoc();
    CHECK_OP(stateInfo, );

    createWigDoc();
    CHECK_OP(stateInfo, );

    bedTask = new SaveDocumentTask(bedDoc);
    addSubTask(bedTask);
    activeSubtasks++;

    wigTask = new SaveDocumentTask(wigDoc);
    addSubTask(wigTask);
    activeSubtasks++;
}

void CEASSupportTask::createBedDoc() {
    QString bedUrl = workingDir + "/" + "tmp.bed";

    DocumentFormat *bedFormat = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::BED);
    CHECK_EXT(NULL != bedFormat, stateInfo.setError("NULL bed format"), );

    bedDoc = bedFormat->createNewLoadedDocument(
        IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), bedUrl, stateInfo);
    CHECK_OP(stateInfo, );
    bedDoc->setDocumentOwnsDbiResources(false);

    AnnotationTableObject *ato = new AnnotationTableObject("bed_anns");
    foreach (const SharedAnnotationData &sad, settings.getBedData()) {
        ato->addAnnotation(new Annotation(sad), QString());
    }
    bedDoc->addObject(ato);
}

void CEASSupportTask::createWigDoc() {
    QString wigUrl = workingDir + "/" + "tmp.wig";

    DocumentFormat *wigFormat = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_TEXT);
    CHECK_EXT(NULL != wigFormat, stateInfo.setError("NULL wig format"), );

    wigDoc = wigFormat->createNewLoadedDocument(
        IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE), wigUrl, stateInfo);
    CHECK_OP(stateInfo, );
    wigDoc->setDocumentOwnsDbiResources(false);

    TextObject *to = new TextObject(settings.getWigData(), "wig_object");
    wigDoc->addObject(to);
}

bool CEASSupportTask::canStartETTask() const {
    return (0 == activeSubtasks);
}

QList<Task*> CEASSupportTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> result;
    CHECK(!subTask->isCanceled(), result);
    CHECK(!subTask->hasError(), result);

    if (bedTask == subTask || wigTask == subTask) {
        activeSubtasks--;
        if (canStartETTask()) {
            settings.getCeasSettings().setBedFile(bedDoc->getURLString());
            settings.getCeasSettings().setWigFile(wigDoc->getURLString());
            QStringList args = settings.getCeasSettings().getArgumentList();

            logParser = new ExternalToolLogParser();
            etTask = new ExternalToolRunTask(CEASSupport::TOOL_NAME, args, logParser, workingDir);
            result << etTask;
        }
    }

    return result;
}

void CEASSupportTask::copyFile(const QString &src, const QString &dst) {
    if (!QFile::exists(src)) {
        setError(tr("Can not find a file: %1").arg(src));
        return;
    }

    QSet<QString> excludeFileNames = DocumentUtils::getNewDocFileNameExcludesHint();
    if (!GUrlUtils::renameFileWithNameRoll(dst, stateInfo, excludeFileNames, &taskLog)) {
        return;
    }

    bool copied = QFile::copy(src, dst);
    if (!copied) {
        setError(tr("Can not copy the result file to: %1").arg(dst));
        return;
    }
}

void CEASSupportTask::run() {
    QString tmpPdfFile = workingDir + "/tmp.pdf";
    copyFile(tmpPdfFile, settings.getCeasSettings().getImageFilePath());
    CHECK_OP(stateInfo, );

    QString tmpXlsFile = workingDir + "/tmp.xls";
    copyFile(tmpXlsFile, settings.getCeasSettings().getAnnsFilePath());
    CHECK_OP(stateInfo, );
}

const CEASTaskSettings & CEASSupportTask::getSettings() const {
    return settings;
}

} // U2
