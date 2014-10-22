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

#include <QCoreApplication>
#include <QDir>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2SequenceDbi.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/UserApplicationsSettings.h>

#include "ExtractProductTask.h"

namespace U2 {

namespace {

QString getProductName(const QString &sequenceName, qint64 sequenceLength, const U2Region &region) {
    qint64 endPos = region.endPos();
    if (endPos > sequenceLength) {
        endPos = endPos % sequenceLength;
    }

    return QString("%1:%2-%3")
        .arg(sequenceName)
        .arg(region.startPos + 1)
        .arg(endPos);
}

}

ExtractProductTask::ExtractProductTask(const InSilicoPcrProduct &product, const U2EntityRef &sequenceRef, const QString &outputFile)
: Task(tr("Extract PCR product"), TaskFlags_FOSE_COSC), product(product), sequenceRef(sequenceRef), outputFile(outputFile), result(NULL)
{

}

ExtractProductTask::~ExtractProductTask() {
    delete result;
}

DNASequence ExtractProductTask::getProductSequence() {
    DNASequence result("", "");
    DbiConnection connection(sequenceRef.dbiRef, stateInfo);
    CHECK_OP(stateInfo, result);
    SAFE_POINT_EXT(NULL != connection.dbi, setError(L10N::nullPointerError("DBI")), result);
    U2SequenceDbi *sequenceDbi = connection.dbi->getSequenceDbi();
    SAFE_POINT_EXT(NULL != sequenceDbi, setError(L10N::nullPointerError("Sequence DBI")), result);

    U2Sequence sequence = sequenceDbi->getSequenceObject(sequenceRef.entityId, stateInfo);
    CHECK_OP(stateInfo, result);

    result.seq = sequenceDbi->getSequenceData(sequenceRef.entityId, product.region, stateInfo);
    CHECK_OP(stateInfo, result);
    if (product.region.endPos() > sequence.length) {
        U2Region tail(0, product.region.endPos() % sequence.length);
        result.seq += sequenceDbi->getSequenceData(sequenceRef.entityId, tail, stateInfo);
        CHECK_OP(stateInfo, result);
    }

    result.setName(getProductName(sequence.visualName, sequence.length, product.region));
    return result;
}

AnnotationData ExtractProductTask::getPrimerAnnotation(const QByteArray &primer, int matchLengh, U2Strand::Direction strand, int sequenceLength) const {
    AnnotationData result;
    U2Region region;
    if (U2Strand::Direct == strand) {
        region = U2Region(0, matchLengh);
    } else {
        region = U2Region(sequenceLength - matchLengh, matchLengh);
    }
    result.location->regions << region;
    result.location->strand = U2Strand(strand);

    result.name = GBFeatureUtils::getKeyInfo(GBFeatureKey_primer).text;
    result.qualifiers << U2Qualifier("sequence", primer);
    return result;
}

void ExtractProductTask::run() {
    IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    SAFE_POINT_EXT(NULL != iof, setError(L10N::nullPointerError("IOAdapterFactory")), );

    DocumentFormat *format = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK);
    SAFE_POINT_EXT(NULL != format, setError(L10N::nullPointerError("Genbank Format")), );
    QScopedPointer<Document> doc(format->createNewLoadedDocument(iof, outputFile, stateInfo));
    CHECK_OP(stateInfo, );

    U2DbiRef dbiRef = AppContext::getDbiRegistry()->getSessionTmpDbiRef(stateInfo);
    CHECK_OP(stateInfo, );

    DNASequence productSequence = getProductSequence();
    CHECK_OP(stateInfo, );
    U2EntityRef productRef = U2SequenceUtils::import(dbiRef, productSequence, stateInfo);
    CHECK_OP(stateInfo, );

    U2SequenceObject *sequenceObject = new U2SequenceObject(productSequence.getName(), productRef);
    doc->addObject(sequenceObject);
    AnnotationTableObject *annotations = new AnnotationTableObject(productSequence.getName() + " features", dbiRef);
    annotations->addAnnotation(getPrimerAnnotation(product.forwardPrimer, product.forwardPimerMatchLength, U2Strand::Direct, productSequence.length()));
    annotations->addAnnotation(getPrimerAnnotation(product.reversePrimer, product.reversePimerMatchLength, U2Strand::Complementary, productSequence.length()));
    annotations->addObjectRelation(GObjectRelation(GObjectReference(sequenceObject), ObjectRole_Sequence));
    doc->addObject(annotations);

    result = doc.take();
}

Document * ExtractProductTask::takeResult() {
    CHECK(NULL != result, NULL);
    if (result->thread() != QCoreApplication::instance()->thread()) {
        result->moveToThread(QCoreApplication::instance()->thread());
    }
    Document *returnValue = result;
    result = NULL;
    return returnValue;
}

/************************************************************************/
/* ExtractProductWrapperTask */
/************************************************************************/
ExtractProductWrapperTask::ExtractProductWrapperTask(const InSilicoPcrProduct &product, const U2EntityRef &sequenceRef, const QString &sequenceName, qint64 sequenceLength)
: Task(tr("Extract PCR product and open document"), TaskFlags_NR_FOSE_COSC)
{
    prepareUrl(product, sequenceName, sequenceLength);
    CHECK_OP(stateInfo, );
    extractTask = new ExtractProductTask(product, sequenceRef, outputFile);
}

void ExtractProductWrapperTask::prepare() {
    addSubTask(extractTask);
}

QList<Task*> ExtractProductWrapperTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> result;
    CHECK(extractTask == subTask, result);
    SaveDocFlags flags;
    flags |= SaveDoc_OpenAfter;
    flags |= SaveDoc_DestroyAfter;
    flags |= SaveDoc_Overwrite;
    QFile::remove(outputFile);
    result << new SaveDocumentTask(extractTask->takeResult(), flags);
    return result;
}

Task::ReportResult ExtractProductWrapperTask::report() {
    if (extractTask->isCanceled()) {
        QFile::remove(outputFile);
    }
    return ReportResult_Finished;
}

void ExtractProductWrapperTask::prepareUrl(const InSilicoPcrProduct &product, const QString &sequenceName, qint64 sequenceLength) {
    // generate file name
    QString fileName = getProductName(sequenceName, sequenceLength, product.region) + ".gb";
    QRegExp regExp("[^A-z0-9_\\-\\s\\.\\(\\)]");
    fileName.replace(regExp, "_");

    // prepare directory
    QString outputDir = AppContext::getAppSettings()->getUserAppsSettings()->getDefaultDataDirPath() + QDir::separator() + "pcr";
    QDir dir(outputDir);
    if (!dir.exists()) {
        bool created = dir.mkpath(outputDir);
        if (!created) {
            setError(tr("Can not create a directory: %1").arg(outputDir));
        }
    }

    QString url = outputDir + QDir::separator() + fileName;
    outputFile = GUrlUtils::rollFileName(url, "_", QSet<QString>());

    // reserve file
    QFile file(outputFile);
    file.open(QIODevice::WriteOnly);
    file.close();
}

} // U2
