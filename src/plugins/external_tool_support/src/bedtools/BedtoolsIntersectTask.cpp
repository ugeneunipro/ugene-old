/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#include "BedtoolsIntersectTask.h"
#include "BedtoolsSupport.h"

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2FeatureUtils.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/UserApplicationsSettings.h>

#include <U2Lang/DbiDataStorage.h>

#include <QDir>
#include <QFileInfo>
#include <QTemporaryFile>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
//BedtoolIntersectTask
BedtoolsIntersectLogParser::BedtoolsIntersectLogParser(const QString &resultFile)
    : ExternalToolLogParser() {
    result.setFileName(resultFile);
    SAFE_POINT( result.open(QIODevice::WriteOnly), "Output file open error", );
    result.close();
}

void BedtoolsIntersectLogParser::parseOutput(const QString &partOfLog) {
    result.open(QIODevice::Append);
    result.write(partOfLog.toLatin1(), partOfLog.size());
    result.close();
}

BedtoolsIntersectTask::BedtoolsIntersectTask(const BedtoolsIntersectFilesSettings &settings)
    : Task(tr("BedtoolsIntersect task"), TaskFlags_NR_FOSE_COSC),
      settings(settings)
{}

void BedtoolsIntersectTask::prepare() {
    CHECK_EXT(!settings.inputA.isEmpty(), setError(tr("No input A URL")), );
    CHECK_EXT(!settings.inputB.isEmpty(), setError(tr("No input B URL(s)")), );
    CHECK_EXT(!settings.out.isEmpty(), setError(tr("Output URL is not set")), );

    for (int i = 0; i < settings.inputB.size(); i++) {
        settings.inputB[i] = QFileInfo(settings.inputB[i]).absoluteFilePath();
        if (settings.inputB.size() != 1 && i != settings.inputB.size() - 1){
            settings.inputB[i] += ",";
        }
    }

    const QStringList args = getParameters();
    ExternalToolLogParser* logParser = new BedtoolsIntersectLogParser(settings.out);
    ExternalToolRunTask* etTask = new ExternalToolRunTask(ET_BEDTOOLS, args, logParser);
    addSubTask(etTask);
}

const QStringList BedtoolsIntersectTask::getParameters() const {
    QStringList res;

    res << "intersect";

    res << "-a" << QFileInfo(settings.inputA).absoluteFilePath();
    res << "-b" << settings.inputB;

    if (settings.reportAbsence) {
        res << "-v";
    } else {
        res << "-f" << QString::number(settings.minOverlap, 'g', 9);
        if (settings.reportFeatures) {
            res << "-wa";
        }
    }

    return res;
}

//////////////////////////////////////////////////////////////////////////
// BedtoolsIntersectAnnotationsByEntityTask
BedtoolsIntersectAnnotationsByEntityTask::BedtoolsIntersectAnnotationsByEntityTask(const BedtoolsIntersectByEntityRefSettings &settings)
    : Task(tr("Intersect annotations task"), TaskFlags_NR_FOSE_COSC),
      settings(settings),
      saveAnnotationsTask(NULL),
      intersectTask(NULL),
      loadResultTask(NULL)
{}

void BedtoolsIntersectAnnotationsByEntityTask::prepare() {
    QList<Document*> docs;

    const QString tmpDir = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath("intersect_annotations") + QDir::separator();
    QDir().mkpath(tmpDir);

    {
        QTemporaryFile *a = new QTemporaryFile(tmpDir + "A-XXXXXX.bed", this);
        QTemporaryFile *b = new QTemporaryFile(tmpDir + "B-XXXXXX.bed", this);

        a->open();
        tmpUrlA = a->fileName();
        a->close();

        b->open();
        tmpUrlB = b->fileName();
        b->close();

        Document* docA = createAnnotationsDocument(tmpUrlA, settings.entitiesA);
        CHECK(docA != NULL, );

        Document* docB = createAnnotationsDocument(tmpUrlB, settings.entitiesB);
        CHECK(docB != NULL, );

        docs << docA << docB;
    }

    saveAnnotationsTask = new SaveMultipleDocuments(docs, false);
    addSubTask(saveAnnotationsTask);
}

QList<Task*> BedtoolsIntersectAnnotationsByEntityTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> res;

    const QString tmpDir = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath("intersect_annotations") + QDir::separator();

    if (subTask == saveAnnotationsTask) {
        QTemporaryFile* outputFile = new QTemporaryFile(tmpDir + "Intersect-XXXXXX.bed", this);
        outputFile->open();
        tmpUrlResult = outputFile->fileName();
        outputFile->close();

        BedtoolsIntersectFilesSettings stngs(tmpUrlA, QStringList() << tmpUrlB, tmpUrlResult,
                                             settings.minOverlap,
                                             settings.reportAbsence,
                                             settings.reportFeatures);
        intersectTask = new BedtoolsIntersectTask(stngs);
        res << intersectTask;
    }

    if (subTask == intersectTask) {
        IOAdapterFactory* ioFactory = IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE);
        CHECK_EXT(ioFactory != NULL, setError(tr("Failed to get IOAdapterFactory")), res);

        loadResultTask = new LoadDocumentTask(BaseDocumentFormats::BED, GUrl(tmpUrlResult), ioFactory);
        res << loadResultTask;
    }
    if (subTask == loadResultTask) {
        Document* resultDoc = loadResultTask->getDocument();
        CHECK_EXT(resultDoc != NULL, setError(tr("Result document is NULL")), res);
        result = resultDoc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
    }
    return res;
}

Document* BedtoolsIntersectAnnotationsByEntityTask::createAnnotationsDocument(const QString &url, const QList<U2EntityRef> &entities) {
    DocumentFormat* bed = BaseDocumentFormats::get(BaseDocumentFormats::BED);
    CHECK_EXT(bed != NULL, setError(tr("Failed to get BED format")), NULL);

    IOAdapterFactory* ioFactory = IOAdapterUtils::get(BaseIOAdapters::LOCAL_FILE);
    CHECK_EXT(ioFactory != NULL, setError(tr("Failed to get IOAdapterFactory")), NULL);

    Document* doc = new Document(bed, ioFactory, GUrl(url), entities.first().dbiRef);

    U2OpStatusImpl os;
    foreach (const U2EntityRef& enRef, entities) {
        U2AnnotationTable t = U2FeatureUtils::getAnnotationTable(enRef, os);
        AnnotationTableObject* table = new AnnotationTableObject(t.visualName, enRef);
        doc->setLoaded(true);
        doc->addObject(table);
    }

    return doc;
}

} // namespace
