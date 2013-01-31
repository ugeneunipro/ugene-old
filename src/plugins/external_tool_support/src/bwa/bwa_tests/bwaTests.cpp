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

#include "bwaTests.h"

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/Log.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/TextObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/GUrlUtils.h>

#include <U2Formats/SAMFormat.h>

#include <U2View/DnaAssemblyUtils.h>

#include <QtCore/QDir>
#include <QtCore/QRegExp>

/* TRANSLATOR U2::GTest*/

namespace U2 {

#define READS_FILE_NAME_ATTR "reads"
#define INDEX_ATTR "index"
#define PATTERN_FILE_NAME_ATTR "pattern"
#define BUILD_INDEX_ATTR "build"
#define NEGATIVE_ATTR "negative"
#define INDEX_ALGORITHM_ATTR "index-algorithm"
#define N_ATTR "n"
#define MAX_GAP_OPENS_ATTR "max-gap-opens"
#define MAX_GAP_EXTENSIONS_ATTR "max-gap-extensions"
#define INDEL_OFFSET_ATTR "indel-offset"
#define MAX_LONG_DELETION_EXTENSIONS_ATTR "max-long-deletion-extensions"
#define SEED_LENGTH_ATTR "seed-length"
#define MAX_SEED_DIFFERENCES_ATTR "max-seed-differences"
#define MAX_QUEUE_ENTRIES_ATTR "max-queue-entries"
#define THREADS_ATTR "threads"
#define MISMATCH_PENALTY_ATTR "mismatch-penalty"
#define GAP_OPEN_PENALTY_ATTR "gap-open-penalty"
#define GAP_EXTENSION_PENALTY_ATTR "gap-extension-penalty"
#define BEST_HITS_ATTR "best-hits"
#define QUALITY_THRESHOLD_ATTR "quality-threshold"
#define BARCODE_LENGTH_ATTR "barcode-length"
#define COLORSPACE_ATTR "colorspace"
#define LONG_SCALED_GAP_PENALTY_FOR_LONG_DELETIONS_ATTR "long-scaled-gap-penalty-for-long-deletions"
#define NON_ITERATIVE_MODE_ATTR "non-iterative-mode"

void GTest_Bwa::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);
    bwaTask = NULL;
    resultLoadTask = NULL;
    indexName = "";
    readsFileName = "";
    patternFileName = "";
    negativeError = "";
    usePrebuildIndex = true;
    subTaskFailed = false;
    indexName = el.attribute(INDEX_ATTR);
    if(indexName.isEmpty()) {failMissingValue(INDEX_ATTR); return;}
    QString buildStr = el.attribute(BUILD_INDEX_ATTR);
    if(!buildStr.isEmpty()) {
        bool ok = false;
        usePrebuildIndex = !buildStr.toInt(&ok);
        if(!ok) {
            failMissingValue(BUILD_INDEX_ATTR);
            return;
        }
    }
    readsFileName = el.attribute(READS_FILE_NAME_ATTR);
    if(readsFileName.isEmpty()) {
        failMissingValue(READS_FILE_NAME_ATTR);
        return;
    }
    patternFileName = el.attribute(PATTERN_FILE_NAME_ATTR);
    if(patternFileName.isEmpty()) {
        failMissingValue(PATTERN_FILE_NAME_ATTR);
        return;
    }
    negativeError = el.attribute(NEGATIVE_ATTR);

    bool ok = false;
    {
        const QString attr = INDEX_ALGORITHM_ATTR;
        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_INDEX_ALGORITHM, el.attribute(attr));
        }
    }
    {
        const QString attr = N_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_N, el.attribute(attr));
        }
    }
    {
        const QString attr = MAX_GAP_OPENS_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_MAX_GAP_OPENS, el.attribute(attr));
        }
    }
    {
        const QString attr = MAX_GAP_EXTENSIONS_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_MAX_GAP_EXTENSIONS, el.attribute(attr).toInt(&ok));
            if(!ok) failMissingValue(attr);
        }
    }
    {
        const QString attr = INDEL_OFFSET_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_INDEL_OFFSET, el.attribute(attr).toInt(&ok));
            if(!ok) failMissingValue(attr);
        }
    }
    {
        const QString attr = MAX_LONG_DELETION_EXTENSIONS_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_MAX_LONG_DELETION_EXTENSIONS, el.attribute(attr).toInt(&ok));
            if(!ok) failMissingValue(attr);
        }
    }
    {
        const QString attr = SEED_LENGTH_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_SEED_LENGTH, el.attribute(attr).toInt(&ok));
            if(!ok) failMissingValue(attr);
        }
    }
    {
        const QString attr = MAX_SEED_DIFFERENCES_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_MAX_SEED_DIFFERENCES, el.attribute(attr).toInt(&ok));
            if(!ok) failMissingValue(attr);
        }
    }
    {
        const QString attr = MAX_QUEUE_ENTRIES_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_MAX_QUEUE_ENTRIES, el.attribute(attr).toInt(&ok));
            if(!ok) failMissingValue(attr);
        }
    }
    {
        const QString attr = THREADS_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_THREADS, el.attribute(attr).toInt(&ok));
            if(!ok) failMissingValue(attr);
        }
    }
    {
        const QString attr = MISMATCH_PENALTY_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_MISMATCH_PENALTY, el.attribute(attr).toInt(&ok));
            if(!ok) failMissingValue(attr);
        }
    }
    {
        const QString attr = GAP_OPEN_PENALTY_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_GAP_OPEN_PENALTY, el.attribute(attr).toInt(&ok));
            if(!ok) failMissingValue(attr);
        }
    }
    {
        const QString attr = GAP_EXTENSION_PENALTY_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_GAP_EXTENSION_PENALTY, el.attribute(attr).toInt(&ok));
            if(!ok) failMissingValue(attr);
        }
    }
    {
        const QString attr = BEST_HITS_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_BEST_HITS, el.attribute(attr).toInt(&ok));
            if(!ok) failMissingValue(attr);
        }
    }
    {
        const QString attr = QUALITY_THRESHOLD_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_QUALITY_THRESHOLD, el.attribute(attr).toInt(&ok));
            if(!ok) failMissingValue(attr);
        }
    }
    {
        const QString attr = BARCODE_LENGTH_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_BARCODE_LENGTH, el.attribute(attr).toInt(&ok));
            if(!ok) failMissingValue(attr);
        }
    }
    {
        const QString attr = COLORSPACE_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_COLORSPACE, true);
        }
    }
    {
        const QString attr = LONG_SCALED_GAP_PENALTY_FOR_LONG_DELETIONS_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_LONG_SCALED_GAP_PENALTY_FOR_LONG_DELETIONS, true);
        }
    }
    {
        const QString attr = NON_ITERATIVE_MODE_ATTR;

        if(!el.attribute(attr).isEmpty()) {
            config.setCustomValue(BwaTask::OPTION_NON_ITERATIVE_MODE, true);
        }
    }
}

void GTest_Bwa::prepare() {
    if(!usePrebuildIndex) {
        QFileInfo refFile(env->getVar("COMMON_DATA_DIR") + "/" + indexName);
        if(!refFile.exists()) {
            stateInfo.setError(QString("file not exist %1").arg(refFile.absoluteFilePath()));
            return;
        }
    }
    QFileInfo readsFile(env->getVar("COMMON_DATA_DIR") + "/" + readsFileName);
    if(!readsFile.exists()) {
        stateInfo.setError(QString("file not exist %1").arg(readsFile.absoluteFilePath()));
        return;
    }

    readsFileUrl = readsFile.absoluteFilePath();

    QFileInfo patternFile(env->getVar("COMMON_DATA_DIR") + "/" + patternFileName);
    if(!patternFile.exists()) {
        stateInfo.setError(QString("file not exist %1").arg(patternFile.absoluteFilePath()));
        return;
    }

    QString tmpDataDir = getTempDataDir();
    if (tmpDataDir.isEmpty()) {
        setError("Can't create tmp data dir!");
        return;
    }

    resultDirPath = tmpDataDir + "/" + QString::number(getTaskId());
    GUrlUtils::prepareDirLocation(resultDirPath,stateInfo);
    if (hasError()) {
        setError("Failed to create result data dir!");
        return;
    }
    
    config.shortReadSets.append(readsFileUrl);
    config.refSeqUrl = GUrl(env->getVar("COMMON_DATA_DIR") + "/" + indexName);
    config.prebuiltIndex = usePrebuildIndex;
    config.resultFileName = GUrl(resultDirPath + "/result.sam");
    config.algName = BwaTask::taskName;
    config.openView = false;
    bwaTask = new BwaTask(config);

    addSubTask(bwaTask);
}

QList<Task*> GTest_Bwa::onSubTaskFinished(Task* subTask) {
    Q_UNUSED(subTask);
    QList<Task*> res;
    if (hasError() || subTask->hasError() || isCanceled()) {
        subTaskFailed = true;
        return res;
    }

    if(subTask == bwaTask) {
        if(bwaTask->hasError()) {
            subTaskFailed = true;
            return res;
        }
        if (!bwaTask->isHaveResult()) {
            subTaskFailed = true;
            bwaTask->setError("Reference assembly failed - no possible alignment found");
            return res;
        }
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(config.resultFileName));
        // SAM format is removed from supported formats list and supported only by BAM plugin
        // Create it manually and provide to LoadDocumentTask
        SAMFormat* samFormat = new SAMFormat();
        resultLoadTask = new LoadDocumentTask(samFormat, config.resultFileName,iof);
        samFormat->setParent(resultLoadTask);
        res << resultLoadTask;

    } else if (subTask == resultLoadTask) {

        Document* doc = resultLoadTask->getDocument();
        CHECK_EXT(doc != NULL, setError("Failed to load result document"), res);
        ma1 =  qobject_cast<MAlignmentObject*> (doc->getObjects().first())->getMAlignment();
        QFileInfo patternFile(env->getVar("COMMON_DATA_DIR")+"/"+patternFileName);
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(patternFile.absoluteFilePath()));

        // SAM format is removed from supported formats list and supported only by BAM plugin
        // Create it manually and provide to LoadDocumentTask
        SAMFormat* samFormat = new SAMFormat();
        patternLoadTask = new LoadDocumentTask(samFormat, patternFile.absoluteFilePath(), iof);
        samFormat->setParent(patternLoadTask);

        patternLoadTask->setSubtaskProgressWeight(0);
        res << patternLoadTask;

    } else if(subTask == patternLoadTask) {
        if(patternLoadTask->hasError()) {
            subTaskFailed = true;
            return res;
        }
        Document *doc = patternLoadTask->getDocument();
        assert(doc!=NULL);

        QList<GObject*> list = doc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
        CHECK_EXT(list.size() > 0, setError(  QString("container of object with type \"%1\" is empty").arg(GObjectTypes::MULTIPLE_ALIGNMENT) ), res);
        MAlignmentObject* ma2Obj = qobject_cast<MAlignmentObject*>(list.first());
        CHECK_EXT(ma2Obj != NULL, setError(QString("Can't cast GObject to MAlignmentObject")), res);
        ma2 = ma2Obj->getMAlignment();
    }
    return res;
}

void GTest_Bwa::run() {

    if(subTaskFailed) {
        return;
    }

    const QList<MAlignmentRow> &alignedSeqs1 = ma1.getRows();
    const QList<MAlignmentRow> &alignedSeqs2 = ma2.getRows();

    if(alignedSeqs1.count() != alignedSeqs2.count()) {
        stateInfo.setError(QString("Aligned sequences number not matched \"%1\", expected \"%2\"").arg(alignedSeqs1.count()).arg(alignedSeqs2.count()));
        return;
    }

    foreach(const MAlignmentRow &maItem1, alignedSeqs1) {
        bool nameFound = false;
        if(maItem1.getName().compare("reference", Qt::CaseInsensitive)==0) continue;
        foreach(const MAlignmentRow &maItem2, alignedSeqs2) {
            if (maItem1.getName() == maItem2.getName()) {
                nameFound = true;
                int l1 = maItem1.getCoreLength();
                int l2 = maItem2.getCoreLength();
                if (l1 != l2) {
                    stateInfo.setError(  QString("Aligned sequences \"%1\" length not matched \"%2\", expected \"%3\"").arg(maItem1.getName()).arg(l1).arg(l2) );
                    return;
                }
                if (maItem1.getCore() != maItem2.getCore()) {
                    stateInfo.setError(  QString("Aligned sequences \"%1\" not matched \"%2\", expected \"%3\"").arg(maItem1.getName()).arg(QString(maItem1.getCore())).arg(QString(maItem2.getCore())) );
                    return;
                }

                //DNAQuality qual1 = maItem1.getCoreQuality();
                //DNAQuality qual2 = maItem1.getCoreQuality();
                //if(qual1.type != qual2.type) {
                //    stateInfo.setError(  QString("Aligned sequences quality type \"%1\" not matched \"%2\", expected \"%3\"").arg(maItem1.getName()).arg(qual1.type).arg(qual2.type) );
                //}
                //if(qual1.qualCodes != qual2.qualCodes) {
                //    stateInfo.setError(  QString("Aligned sequences quality \"%1\" not matched \"%2\", expected \"%3\"").arg(maItem1.getName()).arg(QString(qual1.qualCodes)).arg(QString(qual2.qualCodes)) );
                //    return;
                //}
            }
        }
        if (!nameFound) {
            stateInfo.setError(  QString("aligned sequence not found \"%1\"").arg(maItem1.getName()) );
        }
    }
}

Task::ReportResult GTest_Bwa::report() {
    if(!negativeError.isEmpty()) {
        if(hasSubtasksWithErrors()) {
            return ReportResult_Finished;
        } else {
            setError(QString("Negative test failed: error string is empty, expected error \"%1\"").arg(negativeError));
        }
    } else {
        propagateSubtaskError();
    }
    return ReportResult_Finished;
}

void GTest_Bwa::cleanup() {

    // delete index
    if(!usePrebuildIndex) {
        QString prefix = env->getVar("TEMP_DATA_DIR")+"/"+QString::number(getTaskId());
        QStringList files(QStringList() << prefix+".amb" << prefix+".ann"
                          << prefix+".bwt" << prefix+".pac" << prefix+".rbwt" << prefix+".rpac"
                          << prefix+".rsa" << prefix+".sa");
        foreach(QString file, files) {
            QFileInfo tmpFile(file);
            if(tmpFile.exists()) {
                ioLog.trace(QString("Deleting index file \"%1\"").arg(tmpFile.absoluteFilePath()));
                QFile::remove(tmpFile.absoluteFilePath());
            }
        }
    }
    
    //delete tmp result
    if (QFileInfo(resultDirPath).exists()) {
        ioLog.trace(QString("Deleting tmp result dir %1").arg(resultDirPath));
        GUrlUtils::removeDir(resultDirPath, stateInfo);
    }

    ma1.clear();
    ma2.clear();
}


GTest_Bwa::~GTest_Bwa() {
    cleanup();
}

QString GTest_Bwa::getTempDataDir()
{
    QString dir(env->getVar("TEMP_DATA_DIR"));
    if (!QDir(dir).exists()) {
        bool ok = QDir::root().mkpath(dir);
        if (!ok) {
            return QString();
        }
    }
    return dir;
}

QList<XMLTestFactory*> BwaTests::createTestFactories() {
    QList<XMLTestFactory*> res;
    res.append(GTest_Bwa::createFactory());
    return res;
}

} // namespace U2
