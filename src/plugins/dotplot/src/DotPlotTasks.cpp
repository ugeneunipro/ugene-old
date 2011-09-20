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

#include "DotPlotTasks.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Formats/DocumentFormatUtils.h>

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentFormatConfigurators.h>

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/AddDocumentTask.h>


namespace U2 {

void SaveDotPlotTask::run() {

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    saveDotPlot(stream);

    file.close();
}

void LoadDotPlotTask::run() {

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }

    QTextStream stream(&file);
    if (!loadDotPlot(stream, file.size())) {

        stateInfo.setError(tr("Wrong dotplot format"));
    }

    file.close();
}

// check if the file opens
DotPlotDialogs::Errors SaveDotPlotTask::checkFile(const QString &filename) {

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return DotPlotDialogs::ErrorOpen;
    }

    file.close();
    return DotPlotDialogs::NoErrors;
}

void SaveDotPlotTask::saveDotPlot(QTextStream &stream){

    Q_ASSERT (sequenceX);
    Q_ASSERT (sequenceY);

    stream << sequenceX->getGObjectName() << endl;
    stream << sequenceY->getGObjectName() << endl;

    stream << minLen << " " << identity << endl;

    Q_ASSERT(directList);

    int listSizes = directList->size() + inverseList->size();
    Q_ASSERT (listSizes);

    int i=0;
    foreach(const DotPlotResults &r, *directList) {
        if (stateInfo.cancelFlag) {
            return;
        }

        stream << r.x << " " << r.y << " " << r.len << endl;
        stateInfo.progress = (100*i)/listSizes;

        i++;
    }

    stream << endl << "0 0 0" << endl;

    Q_ASSERT(inverseList);

    foreach(const DotPlotResults &r, *inverseList) {
        if (stateInfo.cancelFlag) {
            return;
        }

        stream << r.x << " " << r.y << " " << r.len << endl;
        stateInfo.progress = (100*i)/listSizes;

        i++;
    }
}

// check if the file opens and sequence names are the same
DotPlotDialogs::Errors LoadDotPlotTask::checkFile(const QString &filename, const QString &seqXName, const QString &seqYName) {

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {

        return DotPlotDialogs::ErrorOpen;
    }

    QTextStream stream(&file);

    QString readedXName;
    QString readedYName;

    readedXName = stream.readLine();
    readedYName = stream.readLine();

    DotPlotDialogs::Errors err = DotPlotDialogs::NoErrors;

    if (seqXName != readedXName || seqYName != readedYName) {
        err = DotPlotDialogs::ErrorNames;
    }

    file.close();

    return err;
}

bool LoadDotPlotTask::loadDotPlot(QTextStream &stream, int fileSize) {

    QString readedXName;
    QString readedYName;

    readedXName = stream.readLine();
    readedYName = stream.readLine();

    // not checking names, just loading dotplot

    int newMinLen, newIdentity;
    QList<DotPlotResults> newDotPlotDirectList;
    QList<DotPlotResults> newDotPlotInverseList;

    stream >> newMinLen >> newIdentity;

    if (newMinLen<2 || newIdentity<50) {
        // wrong format

        return false;
    }

    *inverted = *direct = false;
    bool readingDirect = true;
    long count = 0;
    while (!stream.atEnd() && !stateInfo.cancelFlag) {

        DotPlotResults r;
        stream >> r.x >> r.y >> r.len;
        if (readingDirect && (r.x == 0) && (r.y == 0) && (r.len == 0)) {

            if (count>0) {
                *direct = true;
                count = 0;
            }
            readingDirect = false;
            continue;
        }

        if (readingDirect) {
            newDotPlotDirectList.push_back(r);
        }
        else {
            newDotPlotInverseList.push_back(r);
        }

        Q_ASSERT (stream.device());
        Q_ASSERT (fileSize);

        int streamPos = stream.device()->pos();
        stateInfo.progress = (100*streamPos)/fileSize;

        count++;
    }

    if (!readingDirect && (count>0)) {
        *inverted = true;
    }

    Q_ASSERT(directList);
    directList->clear();
    *directList = newDotPlotDirectList;

    Q_ASSERT(inverseList);
    inverseList->clear();
    *inverseList = newDotPlotInverseList;

    Q_ASSERT(minLen);
    Q_ASSERT(identity);

    *minLen = newMinLen;
    *identity = newIdentity;

    return true;
}


DotPlotLoadDocumentsTask::DotPlotLoadDocumentsTask(QString firstF, int firstG, QString secondF, int secondG, bool view)
: Task(tr("DotPlot loading"), TaskFlags(TaskFlag_NoRun | TaskFlag_FailOnSubtaskCancel)), noView(!view)
{
    firstFile = firstF;
    firstGap = firstG;

    secondFile = secondF;
    secondGap= secondG;
}

void DotPlotLoadDocumentsTask::prepare() {

    // load sequences
    Document *doc = loadFile(firstFile, firstGap);
    if (doc) {
        docs << doc;
    }

    if (hasError()) {
        return;
    }
    doc = loadFile(secondFile, secondGap);
    if (doc) {
        docs << doc;
    }
}

Document *DotPlotLoadDocumentsTask::loadFile(QString inFile, int gapSize) {

    if(inFile == ""){
        return NULL;
    }
    GUrl URL(inFile);

    Project *project = AppContext::getProject();

    Q_ASSERT(project);
    Document *doc = project->findDocumentByURL(URL);

    // document already present in the project
    if (doc) {
        return doc;
    }

    QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(inFile);
    if (formats.isEmpty()) {
        stateInfo.setError(tr("Detecting format error for file %1").arg(inFile));
        return NULL;
    }

    DocumentFormat* format = formats.first().format;
    Q_ASSERT(format);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(URL));

    QVariantMap formatSettings;
    if (gapSize >= 0) {
        QString mergeToken = DocumentReadingMode_SequenceMergeGapSize;
        formatSettings[mergeToken] = gapSize;
    }

    doc = new Document(format, iof, URL, QList<UnloadedObjectInfo>(), formatSettings);
    doc->setUserModLock(false);

    addSubTask(new AddDocumentTask(doc)); // add document to the project
    addSubTask(new LoadUnloadedDocumentTask(doc)); // load document

    return doc;
}

DotPlotLoadDocumentsTask::~DotPlotLoadDocumentsTask() {

    // error while loading documents
    if (hasError()) {
        Project *project = AppContext::getProject();

        // skip added to the project documents
        if (project) {
            QList<Document*> projectDocs = project->getDocuments();

            foreach (Document *doc, projectDocs) {
                docs.removeAll(doc);
            }
        }

        // delete loaded but not added to the project documents
//        qDeleteAll(docs);
        foreach (Document *doc, docs) {
//            docs.removeAll(doc);
            delete doc;
        }
   }
}

} // namespace
