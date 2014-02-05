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

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/U2SafePoints.h>

#include "ConvertFileTask.h"

namespace U2 {

ConvertFileTask::ConvertFileTask(const GUrl &sourceURL, const QString &targetFormat, const QString &workingDir)
: Task(tr("Conversion file to %1").arg(targetFormat), TaskFlags_NR_FOSCOE),
    sourceURL(sourceURL), targetFormat(targetFormat), workingDir(workingDir),
    loadTask(NULL), saveTask(NULL)
{
    if (!workingDir.endsWith("/") && !workingDir.endsWith("\\")) {
        this->workingDir += "/";
    }
}

void ConvertFileTask::prepare() {
    loadTask = LoadDocumentTask::getDefaultLoadDocTask(sourceURL);
    if (NULL == loadTask) {
        setError(tr("Can not load document from url: %1").arg(sourceURL.getURLString()));
        return;
    }
    addSubTask(loadTask);
}

QList<Task*> ConvertFileTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;
    CHECK(!subTask->hasError(), result);
    CHECK(!hasError(), result);

    if (saveTask == subTask) {
        return result;
    }
    SAFE_POINT_EXT(loadTask == subTask, setError("Unknown subtask"), result);

    bool mainThread = false;
    Document *srcDoc = loadTask->getDocument(mainThread);
    SAFE_POINT_EXT(NULL != srcDoc, setError("NULL document"), result);

    DocumentFormatRegistry *dfr = AppContext::getDocumentFormatRegistry();
    DocumentFormat *df = dfr->getFormatById(targetFormat);
    SAFE_POINT_EXT(NULL != df, setError("NULL document format"), result);

    QSet<GObjectType> selectedFormatObjectsTypes = df->getSupportedObjectTypes();
    QSet<GObjectType> inputFormatObjectTypes;
    QListIterator<GObject*> objectsIterator(srcDoc->getObjects());
    while (objectsIterator.hasNext()) {
        GObject *obj = objectsIterator.next();
        inputFormatObjectTypes << obj->getGObjectType();
    }
    inputFormatObjectTypes.intersect(selectedFormatObjectsTypes);
    if (inputFormatObjectTypes.empty()) {
        setError(tr("The formats are not compatible: %1 and %2").arg(srcDoc->getDocumentFormatId()).arg(targetFormat));
        return result;
    }

    QString ext = targetFormat;
    if (!df->getSupportedDocumentFileExtensions().isEmpty()) {
        ext = df->getSupportedDocumentFileExtensions().first();
    }
    QString fileName = srcDoc->getName() + "." + ext;
    targetUrl = GUrlUtils::rollFileName(workingDir + fileName, QSet<QString>());

    IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(srcDoc->getURL()));
    Document *dstDoc = srcDoc->getSimpleCopy(df, iof, srcDoc->getURL());

    saveTask = new SaveDocumentTask(dstDoc, iof, targetUrl);
    result << saveTask;
    return result;
}

GUrl ConvertFileTask::getSourceURL() const {
    return sourceURL;
}

QString ConvertFileTask::getResult() const {
    return targetUrl;
}

} // U2
