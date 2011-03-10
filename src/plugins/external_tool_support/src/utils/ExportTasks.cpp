#include "ExportTasks.h"

#include <U2Core/DNAAlphabet.h>
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

#include <U2Core/DNASequenceObject.h>
#include <U2Core/MAlignmentObject.h>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
// DNAExportAlignmentTask
SaveAlignmentTask::SaveAlignmentTask(const MAlignment& _ma, const QString& _fileName, DocumentFormatId _f)
: Task("", TaskFlag_None), ma(_ma), fileName(_fileName), format(_f)
{
    GCOUNTER( cvar, tvar, "ExportAlignmentTask" );
    setTaskName(tr("Export alignment to '%1'").arg(QFileInfo(fileName).fileName()));
    setVerboseLogMode(true);

    assert(!ma.isEmpty());
}

void SaveAlignmentTask::run() {
    DocumentFormatRegistry* r = AppContext::getDocumentFormatRegistry();
    DocumentFormat* f = r->getFormatById(format);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(fileName));
    doc.reset(f->createNewDocument(iof, fileName));
    doc->addObject(new MAlignmentObject(ma));
    f->storeDocument(doc.get(), stateInfo);
}


//////////////////////////////////////////////////////////////////////////
// export alignment  2 sequence format

SaveMSA2SequencesTask::SaveMSA2SequencesTask(const MAlignment& _ma, const QString& _url, bool _trimAli, DocumentFormatId _format)
: Task(tr("Export alignment to sequence: %1").arg(_url), TaskFlag_None),
ma(_ma), url(_url), trimAli(_trimAli), format(_format)
{
    GCOUNTER( cvar, tvar, "ExportMSA2SequencesTask" );
    setVerboseLogMode(true);
}

void SaveMSA2SequencesTask::run() {
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

}//namespace

