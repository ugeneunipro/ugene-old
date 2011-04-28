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

#include "ExportTasks.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNATranslation.h>
#include <U2Core/DNATranslationImpl.h>
#include <U2Formats/SCFFormat.h>
#include <U2Core/Counter.h>
#include <U2Core/DNAChromatogramObject.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/TextUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Gui/OpenViewTask.h>
#include <U2Core/MAlignmentObject.h>

namespace U2 {


//////////////////////////////////////////////////////////////////////////
// AddDocumentAndOpenViewTask

AddDocumentAndOpenViewTask::AddDocumentAndOpenViewTask(AbstractExportTask* t) 
: Task("Export sequence to document", TaskFlags_NR_FOSCOE)
{
    exportTask = t;
    addSubTask(exportTask);
}


QList<Task*> AddDocumentAndOpenViewTask::onSubTaskFinished( Task* subTask ) {
    QList<Task*> subTasks;
    if (subTask == exportTask && !subTask->hasError()) {
        Document* doc = exportTask->getDocument();
        const GUrl& fullPath = doc->getURL();
        Project* prj = AppContext::getProject();
        if (prj) {
            Document* sameURLdoc = prj->findDocumentByURL(fullPath);
            if (sameURLdoc) {
                taskLog.trace(tr("Document is already added to the project %1").arg(doc->getURL().getURLString()));
                return subTasks;
            }
        }
        DocumentFormat* format = doc->getDocumentFormat();
        IOAdapterFactory * iof = doc->getIOAdapterFactory();
        Document* clonedDoc = new Document(format, iof, fullPath);
        clonedDoc->loadFrom(doc); // doc was loaded in a separate thread -> clone all GObjects
        assert(!clonedDoc->isTreeItemModified());
        assert(clonedDoc->isLoaded());
        if (!clonedDoc->isStateLocked()) {
            clonedDoc->setModified(doc->isModified());
        }
        subTasks.append(new AddDocumentTask(clonedDoc));
        subTasks.append(new LoadUnloadedDocumentAndOpenViewTask(clonedDoc));           
    }
    //TODO: provide a report if subtask fails
    return subTasks;
}

//////////////////////////////////////////////////////////////////////////
// DNAExportAlignmentTask
ExportAlignmentTask::ExportAlignmentTask(const MAlignment& _ma, const QString& _fileName, DocumentFormatId _f)
: AbstractExportTask("", TaskFlag_None), ma(_ma), fileName(_fileName), format(_f)
{
    GCOUNTER( cvar, tvar, "ExportAlignmentTask" );
    setTaskName(tr("Export alignment to '%1'").arg(QFileInfo(fileName).fileName()));
    setVerboseLogMode(true);

    assert(!ma.isEmpty());
}

void ExportAlignmentTask::run() {
    DocumentFormatRegistry* r = AppContext::getDocumentFormatRegistry();
    DocumentFormat* f = r->getFormatById(format);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(fileName));
    doc.reset(f->createNewDocument(iof, fileName));
    doc->addObject(new MAlignmentObject(ma));
    f->storeDocument(doc.get(), stateInfo);
}


//////////////////////////////////////////////////////////////////////////
// export alignment  2 sequence format

ExportMSA2SequencesTask::ExportMSA2SequencesTask(const MAlignment& _ma, const QString& _url, bool _trimAli, DocumentFormatId _format) 
: AbstractExportTask(tr("Export alignment to sequence: %1").arg(_url), TaskFlag_None), 
ma(_ma), url(_url), trimAli(_trimAli), format(_format)
{
    GCOUNTER( cvar, tvar, "ExportMSA2SequencesTask" );
    setVerboseLogMode(true);
}

void ExportMSA2SequencesTask::run() {
    DocumentFormatRegistry* r = AppContext::getDocumentFormatRegistry();
    DocumentFormat* f = r->getFormatById(format);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(url));
    doc.reset(f->createNewDocument(iof, url));

    QList<DNASequence> lst = MSAUtils::ma2seq(ma, trimAli);
    QSet<QString> usedNames;
    foreach(const DNASequence& s, lst) {
        QString name = s.getName();
        if (usedNames.contains(name)) {
            name = TextUtils::variate(name, " ", usedNames, false, 1);
        }
        doc->addObject(new DNASequenceObject(name, s));
        usedNames.insert(name);
    }
    f->storeDocument(doc.get(), stateInfo);
}

//////////////////////////////////////////////////////////////////////////
// export nucleic alignment 2 amino alignment

ExportMSA2MSATask::ExportMSA2MSATask(const MAlignment& _ma, int _offset, int _len, const QString& _url,
    const QList<DNATranslation*>& _aminoTranslations, DocumentFormatId _format) 
: AbstractExportTask(tr("Export alignment to alignment: %1").arg(_url), TaskFlag_None), 
ma(_ma), offset(_offset), len(_len), url(_url), aminoTranslations(_aminoTranslations), format(_format)
{
    GCOUNTER( cvar, tvar, "ExportMSA2MSATask" );
    setVerboseLogMode(true);
}

void ExportMSA2MSATask::run() {
    DocumentFormatRegistry* r = AppContext::getDocumentFormatRegistry();
    DocumentFormat* f = r->getFormatById(format);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(url));
    doc.reset(f->createNewDocument(iof, url));

    QList<DNASequence> lst = MSAUtils::ma2seq(ma, true);
    QList<GObject*> res;
    for (int i = offset; i < offset + len; i++) {    
        DNASequence& s = lst[i];
        QString name = s.getName();
        if (!aminoTranslations.isEmpty()) {
            DNATranslation* aminoTT = aminoTranslations.first();
            name += "(translated)";

            QByteArray seq = s.seq;
            int len = seq.length() / 3;
            QByteArray resseq(len, '\0');
            if (resseq.isNull() && len != 0) {
                stateInfo.setError( tr("Out of memory") );
                return;
            }
            assert(aminoTT->isThree2One());
            aminoTT->translate(seq.constData(), seq.length(), resseq.data(), resseq.length());

            resseq.replace("*","X");
            DNASequence rs(name, resseq, aminoTT->getDstAlphabet());
            res << new DNASequenceObject(name, rs);
        } else {
            res << new DNASequenceObject(name, s);
        }                
    }
    QString err;
    MAlignment ma = MSAUtils::seq2ma(res, err);
    if (!err.isEmpty()) {
        stateInfo.setError(err);
        return;
    }
    doc->addObject(new MAlignmentObject(ma));
    f->storeDocument(doc.get(), stateInfo);
}

//////////////////////////////////////////////////////////////////////////
// export chromatogram to SCF

ExportDNAChromatogramTask::ExportDNAChromatogramTask( DNAChromatogramObject* _obj, const ExportChromatogramTaskSettings& _settings)
 : AbstractExportTask(tr("Export chromatogram to SCF"), TaskFlags_NR_FOSCOE), 
   cObj(_obj), settings(_settings), loadTask(NULL)
{
    GCOUNTER( cvar, tvar, "ExportDNAChromatogramTask" );
    setVerboseLogMode(true);
}

void ExportDNAChromatogramTask::prepare()
{
    Document* d = cObj->getDocument();
    assert(d != NULL);
    if (d == NULL ) {
        stateInfo.setError("Chromatogram object document is not found!");
        return;
    }

    QList<GObjectRelation> relatedObjs = cObj->findRelatedObjectsByRole(GObjectRelationRole::SEQUENCE);
    assert(relatedObjs.count() == 1);
    if (relatedObjs.count() != 1) {
        stateInfo.setError("Sequence related to chromatogram is not found!");
    }
    QString seqObjName = relatedObjs.first().ref.objName;

    GObject* resObj = d->findGObjectByName(seqObjName);
    DNASequenceObject * sObj = qobject_cast<DNASequenceObject*>(resObj);
    assert(sObj != NULL);

    DNAChromatogram cd = cObj->getChromatogram();
    DNASequence dna = sObj->getSequence();
    
    
    

    if (settings.reverse) {
        TextUtils::reverse(dna.seq.data(), dna.seq.length());
        reverseVector(cd.A);
        reverseVector(cd.C);
        reverseVector(cd.G);
        reverseVector(cd.T);
        int offset = 0;
        if (cObj->getDocument()->getDocumentFormatId() == BaseDocumentFormats::ABIF) {
            int baseNum = cd.baseCalls.count();
            int seqLen = cd.seqLength;
            // this is required for base <-> peak correspondence 
            if (baseNum > seqLen) {
                cd.baseCalls.remove(baseNum - 1);
                cd.prob_A.remove(baseNum - 1);
                cd.prob_C.remove(baseNum - 1);
                cd.prob_G.remove(baseNum - 1);
                cd.prob_T.remove(baseNum - 1);
            }
        } else if (cObj->getDocument()->getDocumentFormatId() == BaseDocumentFormats::SCF) {
            // SCF format particularities
            offset = -1;
        }

        for (int i = 0; i < cd.seqLength; ++i) {
            cd.baseCalls[i] = cd.traceLength - cd.baseCalls[i] + offset;
        } 
        reverseVector(cd.baseCalls);
        reverseVector(cd.prob_A);
        reverseVector(cd.prob_C);
        reverseVector(cd.prob_G);
        reverseVector(cd.prob_T);
    }

    if (settings.complement) {
        DNATranslation* tr = AppContext::getDNATranslationRegistry()->lookupTranslation(BaseDNATranslationIds::NUCL_DNA_DEFAULT_COMPLEMENT);
        tr->translate(dna.seq.data(), dna.length());
        qSwap(cd.A,cd.T);
        qSwap(cd.C, cd.G);
        qSwap(cd.prob_A, cd.prob_T);
        qSwap(cd.prob_C, cd.prob_G);
    
    }
    
    SCFFormat::exportDocumentToSCF(settings.url, cd, dna, stateInfo);
    if (stateInfo.hasError()) {
        return;
    }

    if (settings.loadDocument) {
        IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        loadTask = new LoadDocumentTask(BaseDocumentFormats::SCF, settings.url, iof );
        addSubTask( loadTask );
    }
    
}

Document* ExportDNAChromatogramTask::getDocument() const
{
    Document* doc = loadTask->getDocument();
    return doc;
}

}//namespace

