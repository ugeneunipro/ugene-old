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

#include "FindAlgorithmTask.h"

#include <U2Core/TextUtils.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/Counter.h>
#include <U2Core/AppResources.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/DNASequenceObject.h>

#include <U2Core/DocumentModel.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/DocumentUtils.h>
#include <U2Formats/RawDNASequenceFormat.h>
#include <U2Formats/FastaFormat.h>
#include <U2Formats/GenbankFeatures.h>
#include <U2Core/IOAdapterUtils.h>

namespace U2 {

class StrandContext;

FindAlgorithmTask::FindAlgorithmTask(const FindAlgorithmTaskSettings& s) 
: Task (tr("Find in sequence task"), TaskFlag_None), config(s), currentPos(0),currentLen(0)
{
    GCOUNTER(cvar, tvar, "FindAlgorithmTask");
    tpm = Progress_Manual;
    complementRun = false;
    currentPos = s.searchRegion.startPos;
    assert(config.strand == FindAlgorithmStrand_Direct || config.complementTT!=NULL);

    addTaskResource(TaskResourceUsage(RESOURCE_MEMORY,
        FindAlgorithm::estimateRamUsageInMbytes(config.patternSettings, NULL != config.proteinTT,
        config.pattern.length(), config.maxErr), true));
}

void FindAlgorithmTask::run() {
    FindAlgorithm::find(dynamic_cast<FindAlgorithmResultsListener*>(this),
        config.proteinTT,
        config.complementTT,
        config.strand,
        config.patternSettings,
        config.useAmbiguousBases,
        config.sequence.constData(),
        config.sequence.size(),
        config.searchRegion,
        config.pattern.constData(),
        config.pattern.length(),
        config.singleShot,
        config.maxErr,
        config.maxRegExpResult,
        stateInfo.cancelFlag,
        stateInfo.progress,
        currentPos,
        currentLen);
}

void FindAlgorithmTask::onResult(const FindAlgorithmResult& r) {
    if(newResults.size() >= config.maxResult2Find){
        stateInfo.cancelFlag = true;
        return;
    }
    lock.lock();
    newResults.append(r);
    lock.unlock();
}

QList<FindAlgorithmResult> FindAlgorithmTask::popResults() {
    lock.lock();
    QList<FindAlgorithmResult> res = newResults;
    newResults.clear();
    lock.unlock();
    return res;
}

//////////////////////////////////////////////////////////////////////////
//LoadPatternsFileTask
LoadPatternsFileTask::LoadPatternsFileTask( const QString& _filePath )
: Task("Load pattern from file", TaskFlag_None), filePath(_filePath)
{

}

Document * LoadPatternsFileTask::getDocumentFromFilePath()
{
    GUrl fileUrl(filePath);
    Project *project = AppContext::getProject();
    Document *doc = NULL;
    if(project != NULL){
        Document *doc = project->findDocumentByURL(filePath);

        // document already present in the project
        if (NULL != doc) {
            return doc;
        }
    }

    QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(filePath);
    if (formats.isEmpty()) {
        stateInfo.setError(tr("Detecting format error for file %1").arg(filePath));
        return NULL;
    }

    DocumentFormat *format = formats.first().format;
    if(NULL != dynamic_cast<RawDNASequenceFormat *>(format)) {
        isRawSequence = true;
        return NULL;
    }
    Q_ASSERT(format);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(fileUrl));

    QVariantMap hints;
    doc = format->loadDocument(iof, fileUrl, hints, stateInfo);

    CHECK_OP(stateInfo, NULL);

    return doc;
}

void LoadPatternsFileTask::run()
{
    typedef QPair<QString, QString> NamePattern;

    Document *doc = getDocumentFromFilePath();
    if(NULL != doc && isRawSequence) {
        const QList<GObject *> &objectsFromDoc = doc->findGObjectByType(GObjectTypes::SEQUENCE);

        foreach(GObject *object, objectsFromDoc) {
            U2SequenceObject *sequenceObject = qobject_cast<U2SequenceObject*>(object);
            assert(NULL != sequenceObject);
            QByteArray sequence = sequenceObject->getWholeSequenceData();
            QString seqName = sequenceObject->getSequenceName();
            namesPatterns.append(qMakePair(seqName, QString(sequence)));
        }
    } else {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly))
            setError(QString("Cannot open a file: %1").arg(filePath));

        QTextStream stream(&file);
        int fileSize = file.size();

        while(!stream.atEnd() && !stateInfo.cancelFlag) {
            int streamPos = stream.device()->pos();
            stateInfo.progress = 100 * streamPos / fileSize;
            QString pattern = stream.readLine();
            if (!pattern.isEmpty()){
                bool contains = false;
                foreach(const NamePattern& namePattern, namesPatterns){
                    if (namePattern.second == pattern){
                        contains = true;
                        break;
                    }
                }
                if (!contains){
                    namesPatterns.append(qMakePair(QString(""), pattern));
                }

            }
        }
        file.close();
    }
}

} //namespace



