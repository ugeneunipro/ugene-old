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

#include "CoreLib.h"
#include "GenericReadWorker.h"

#include <U2Core/AnnotationData.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequence.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/FailTask.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/Log.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/QVariantUtils.h>
#include <U2Core/TextObject.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/EMBLGenbankAbstractDocument.h>
#include <U2Formats/GFFFormat.h>

#include <U2Lang/BaseAttributes.h>
#include <U2Lang/BaseSlots.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/Dataset.h>
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowUtils.h>

#include "DocWorkers.h"

namespace U2 {
namespace LocalWorkflow {

static int ct = 0;

/*************************************
 * TextReader
 *************************************/
void TextReader::init() {
    QList<Dataset> sets = actor->getParameter(BaseAttributes::URL_IN_ATTRIBUTE().getId())->getAttributeValue< QList<Dataset> >(context);
    urls = new DatasetFilesIterator(sets);

    assert(ports.size() == 1);
    ch = ports.values().first();
}

void TextReader::sendMessage(const QByteArray &data) {
    QVariantMap m; 
    m[BaseSlots::TEXT_SLOT().getId()] = QString(data);
    m[BaseSlots::URL_SLOT().getId()] = url;
    m[BaseSlots::DATASET_SLOT().getId()] = urls->getLastDatasetName();
    ch->put( Message(mtype, m));
}

Task * TextReader::tick() {
    if(io && io->isOpen()) {
        QByteArray buf;
        buf.resize(1024);
        buf.fill(0);
        int read = io->readLine(buf.data(), 1024);
        buf.resize(read);
        sendMessage(buf);
        if(io->isEof()) {
            io->close();
        }
    } else if (urls->hasNext()) {
        url = urls->getNextFile();
        IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
        io = iof->createIOAdapter();
        if(!io->open(url,IOAdapterMode_Read)) {
            return new FailTask(tr("Can't load file %1").arg(url));
        }
        if(actor->getParameter(BaseAttributes::READ_BY_LINES_ATTRIBUTE().getId())->getAttributeValue<bool>(context) == false) {
            QByteArray buf;
            int read = 0;
            int offs = 0;
            static const int READ_BLOCK_SZ = 1024;
            buf.resize(READ_BLOCK_SZ);
            buf.fill(0);
            do {
                read = io->readBlock( buf.data() + offs, READ_BLOCK_SZ );
                if (read == -1){
                    return new FailTask(tr("Can't load file %1. %2").arg(url).arg(io->errorString()));
                }
                if( read != READ_BLOCK_SZ ) {
                    assert(read < READ_BLOCK_SZ);
                    buf.resize(buf.size() - READ_BLOCK_SZ + read);
                    break;
                }
                offs += read;
                buf.resize( offs + READ_BLOCK_SZ );
            } while(read == READ_BLOCK_SZ);
            
            sendMessage(buf);
            io->close();
        } else {
            QByteArray buf;
            buf.resize(1024);
            buf.fill(0);
            int read = io->readLine(buf.data(), 1024);
            buf.resize(read);
            sendMessage(buf);
            if(io->isEof()) {
                io->close();
            }
        }
    }
    if (!urls->hasNext() && (!io || !io->isOpen())) {
        ch->setEnded();
        setDone();
    }
    return NULL;
}

void TextReader::doc2data(Document* doc) {
    algoLog.info(tr("Reading text from %1").arg(doc->getURLString()));
    foreach(GObject* go, GObjectUtils::select(doc->getObjects(), GObjectTypes::TEXT, UOF_LoadedOnly)) {
        TextObject* txtObject = qobject_cast<TextObject*>(go);
        assert(txtObject);
        QVariantMap m; 
        m[BaseSlots::TEXT_SLOT().getId()] = txtObject->getText();
        m[BaseSlots::URL_SLOT().getId()] = doc->getURLString();
        cache << Message(mtype, m);
    }
}

/*************************************
* TextWriter
*************************************/
void TextWriter::data2doc(Document* doc, const QVariantMap& data) {
    QStringList list = data.value(BaseSlots::TEXT_SLOT().getId()).toStringList();
    QString text = list.join("\n");
    TextObject* to = qobject_cast<TextObject*>(GObjectUtils::selectOne(doc->getObjects(), GObjectTypes::TEXT, UOF_LoadedOnly));
    if (!to) {
        to = new TextObject(text, QString("Text %1").arg(++ct));
        doc->addObject(to);
    } else {
        to->setText(to->getText() + "\n" + text);
    }
}

bool TextWriter::hasDataToWrite(const QVariantMap &data) const {
    return data.contains(BaseSlots::TEXT_SLOT().getId());
}

bool TextWriter::isStreamingSupport() const {
    return false;
}

bool TextWriter::isSupportedSeveralMessages() const {
    return true;
}

/**
 * It can change sequence name for setting unique object name
 */
static U2SequenceObject *addSeqObject(Document *doc, DNASequence &seq) {
    SAFE_POINT(NULL != seq.alphabet, "Add sequence to document: empty alphabet", NULL);
    SAFE_POINT(0 != seq.length(), "Add sequence to document: empty length", NULL);

    if (doc->findGObjectByName(seq.getName())) {
        QString uniqueName = BaseDocWriter::getUniqueObjectName(doc, seq.getName());
        seq.setName(uniqueName);
    }
    algoLog.trace(QString("Adding seq [%1] to %3 doc %2").arg(seq.getName()).arg(doc->getURLString()).arg(doc->getDocumentFormat()->getFormatName()));

    U2SequenceObject *dna = NULL;
    if (doc->getDocumentFormat()->isObjectOpSupported(doc, DocumentFormat::DocObjectOp_Add, GObjectTypes::SEQUENCE)) {
        U2OpStatus2Log os;
        U2EntityRef seqRef = U2SequenceUtils::import(doc->getDbiRef(), seq, os);
        CHECK_OP(os, NULL);
        dna = new U2SequenceObject(seq.getName(), seqRef);
        doc->addObject(dna);
    } else {
        algoLog.trace("Failed to add sequence object to document: op is not supported!");
    }

    return dna;
}

/*************************************
* FastaWriter
*************************************/
void FastaWriter::data2doc(Document* doc, const QVariantMap& data) {
    data2document(doc, data, context, numSplitSequences, currentSplitSequence);
    currentSplitSequence++;
}

bool FastaWriter::hasDataToWrite(const QVariantMap &data) const {
    return SeqWriter::hasSequence(data);
}

void FastaWriter::storeEntry(IOAdapter *io, const QVariantMap &data, int entryNum) {
    streamingStoreEntry(format, io, data, context, entryNum);
    currentSplitSequence++;
}

U2Region FastaWriter::getSplitRegion(int numSplitSequences, int currentSplitSequence, qint64 seqLen) {
    U2Region result;
    result.startPos = currentSplitSequence * (seqLen / numSplitSequences);
    result.length = seqLen / numSplitSequences;
    if (currentSplitSequence == (numSplitSequences - 1)) {
        result.length += seqLen % numSplitSequences;
    }
    return result;
}

static U2SequenceObject * getSeqObj(const QVariantMap &data, WorkflowContext *context, U2OpStatus &os) {
    if (!data.contains(BaseSlots::DNA_SEQUENCE_SLOT().getId())) {
        os.setError("Fasta writer: no sequence");
        return NULL;
    }
    SharedDbiDataHandler seqId = data[BaseSlots::DNA_SEQUENCE_SLOT().getId()].value<SharedDbiDataHandler>();
    U2SequenceObject *seqObj = StorageUtils::getSequenceObject(context->getDataStorage(), seqId);
    if (NULL == seqObj) {
        os.setError("Fasta writer: NULL sequence object");
    }
    return seqObj;
}

void FastaWriter::data2document(Document* doc, const QVariantMap& data, WorkflowContext *context, int numSplitSequences, int currentSplitSequence ) {
    U2OpStatusImpl os;
    std::auto_ptr<U2SequenceObject> seqObj(getSeqObj(data, context, os));
    SAFE_POINT_OP(os, );

    U2Region splitRegion = getSplitRegion(numSplitSequences, currentSplitSequence, seqObj->getSequenceLength());
    QByteArray splitSequence = seqObj->getSequenceData(splitRegion);

    DNASequence seq(seqObj->getSequenceName() + ((numSplitSequences == 1) ? QString("%1..%2").arg(splitRegion.startPos + 1, splitRegion.length) : ""), splitSequence, seqObj->getAlphabet());
    seq.circular = seqObj->isCircular();
    seq.quality = seqObj->getQuality();
    seq.info = seqObj->getSequenceInfo();

    QString sequenceName = data.value(BaseSlots::FASTA_HEADER_SLOT().getId()).toString();
    if (sequenceName.isEmpty()) {
        sequenceName = seq.getName();
        if (sequenceName.isEmpty()) {
            sequenceName = QString("unknown sequence %1").arg(doc->getObjects().size());
        }
    } else {
        seq.info.insert(DNAInfo::FASTA_HDR, sequenceName);
    }
    seq.setName(sequenceName);
    addSeqObject(doc, seq);    
}

inline static U2SequenceObject * getCopiedSequenceObject(const QVariantMap &data, WorkflowContext *context, U2OpStatus2Log &os, const U2Region &reg = U2_REGION_MAX) {
    std::auto_ptr<U2SequenceObject> seqObj(getSeqObj(data, context, os));
    SAFE_POINT_OP(os, NULL);

    SharedDbiDataHandler seqId = data[BaseSlots::DNA_SEQUENCE_SLOT().getId()].value<SharedDbiDataHandler>();
    int refCount = seqId.constData()->getReferenceCount();
    if (refCount > 2) { // need to copy because it is used by another worker
        DNASequence seq = seqObj->getSequence(reg);
        U2EntityRef seqRef = U2SequenceUtils::import(seqObj->getEntityRef().dbiRef, seq, os);
        CHECK_OP(os, NULL);

        return new U2SequenceObject(seqObj->getSequenceName(), seqRef);
    } else {
        return seqObj.release();
    }
}

void FastaWriter::streamingStoreEntry(DocumentFormat* format, IOAdapter *io, const QVariantMap &data, WorkflowContext *context, int entryNum) {
    CHECK(data.contains(BaseSlots::DNA_SEQUENCE_SLOT().getId()), );
    U2OpStatus2Log os;
    std::auto_ptr<U2SequenceObject> seqObj(getCopiedSequenceObject(data, context, os));
    SAFE_POINT_OP(os, );

    QString sequenceName = data.value(BaseSlots::FASTA_HEADER_SLOT().getId(), QString()).toString();
    if (sequenceName.isEmpty()) {
        sequenceName = seqObj->getGObjectName();
        if (sequenceName.isEmpty()) {
            sequenceName = QString("unknown sequence %1").arg(entryNum);
        }
    } else {
        QVariantMap info = seqObj->getSequenceInfo();
        info.insert(DNAInfo::FASTA_HDR, sequenceName);
        seqObj->setSequenceInfo(info);
    }
    seqObj->setGObjectName(sequenceName);

    QMap< GObjectType, QList<GObject*> > objectsMap;
    {
        QList<GObject*> seqs; seqs << seqObj.get();
        objectsMap[GObjectTypes::SEQUENCE] = seqs;
    }
    format->storeEntry(io, objectsMap, os);
}

/*************************************
* FastQWriter
*************************************/
void FastQWriter::data2doc(Document* doc, const QVariantMap& data) {
    data2document(doc, data, context);
}

void FastQWriter::storeEntry(IOAdapter *io, const QVariantMap &data, int entryNum) {
    streamingStoreEntry(format, io, data, context, entryNum);
}

void FastQWriter::data2document(Document* doc, const QVariantMap& data, WorkflowContext *context) {
    CHECK(data.contains(BaseSlots::DNA_SEQUENCE_SLOT().getId()), );
    SharedDbiDataHandler seqId = data[BaseSlots::DNA_SEQUENCE_SLOT().getId()].value<SharedDbiDataHandler>();
    std::auto_ptr<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));
    SAFE_POINT(NULL != seqObj.get(), tr("Fastq writer: NULL sequence object"), );

    DNASequence seq = seqObj->getWholeSequence();

    if (seq.getName().isEmpty()) {
        seq.setName(QString("unknown sequence %1").arg(doc->getObjects().size()));
    }
    addSeqObject(doc, seq);
}

bool FastQWriter::hasDataToWrite(const QVariantMap &data) const {
    return SeqWriter::hasSequence(data);
}

void FastQWriter::streamingStoreEntry(DocumentFormat* format, IOAdapter *io, const QVariantMap &data, WorkflowContext *context, int entryNum) {
    CHECK(data.contains(BaseSlots::DNA_SEQUENCE_SLOT().getId()), );
    U2OpStatus2Log os;
    std::auto_ptr<U2SequenceObject> seqObj(getCopiedSequenceObject(data, context, os));
    SAFE_POINT_OP(os, );

    if (seqObj->getGObjectName().isEmpty()) {
        seqObj->setGObjectName(QString("unknown sequence %1").arg(entryNum));
    }

    QMap< GObjectType, QList<GObject*> > objectsMap;
    {
        QList<GObject*> seqs; seqs << seqObj.get();
        objectsMap[GObjectTypes::SEQUENCE] = seqs;
    }
    format->storeEntry(io, objectsMap, os);
}

/*************************************
 * RawSeqWriter
 *************************************/
void RawSeqWriter::data2doc(Document* doc, const QVariantMap& data) {
    data2document(doc, data, context);
}

void RawSeqWriter::storeEntry(IOAdapter *io, const QVariantMap &data, int entryNum) {
    streamingStoreEntry(format, io, data, context, entryNum);
}

// same as FastQWriter::data2document
void RawSeqWriter::data2document(Document* doc, const QVariantMap& data, WorkflowContext *context) {
    CHECK(data.contains(BaseSlots::DNA_SEQUENCE_SLOT().getId()), );
    SharedDbiDataHandler seqId = data[BaseSlots::DNA_SEQUENCE_SLOT().getId()].value<SharedDbiDataHandler>();
    std::auto_ptr<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));
    SAFE_POINT(NULL != seqObj.get(), tr("Raw sequence writer: NULL sequence object"), );

    DNASequence seq = seqObj->getWholeSequence();

    if (seq.getName().isEmpty()) {
        seq.setName(QString("unknown sequence %1").arg(doc->getObjects().size()));
    }
    addSeqObject(doc, seq);
}

bool RawSeqWriter::hasDataToWrite(const QVariantMap &data) const {
    return SeqWriter::hasSequence(data);
}

void RawSeqWriter::streamingStoreEntry(DocumentFormat* format, IOAdapter *io, const QVariantMap &data, WorkflowContext *context, int) {
    CHECK(data.contains(BaseSlots::DNA_SEQUENCE_SLOT().getId()), );
    U2OpStatus2Log os;
    std::auto_ptr<U2SequenceObject> seqObj(getCopiedSequenceObject(data, context, os));
    SAFE_POINT_OP(os, );

    QMap< GObjectType, QList<GObject*> > objectsMap;
    {
        QList<GObject*> seqs; seqs << seqObj.get();
        objectsMap[GObjectTypes::SEQUENCE] = seqs;
    }
    format->storeEntry(io, objectsMap, os);
}

/*************************************
 * GenbankWriter
 *************************************/
inline static QString getAnnotationName(const QString &seqName) {
    QString result = seqName;
    if (result.contains(SEQUENCE_TAG)) {
        result.replace(SEQUENCE_TAG, FEATURES_TAG);
    } else {
        result += FEATURES_TAG;
    }

    return result;
}

void GenbankWriter::data2doc(Document* doc, const QVariantMap& data) {
    data2document(doc, data, context);
}

void GenbankWriter::storeEntry(IOAdapter *io, const QVariantMap &data, int entryNum) {
    streamingStoreEntry(format, io, data, context, entryNum);
}

void GenbankWriter::data2document(Document* doc, const QVariantMap& data, WorkflowContext *context) {
    std::auto_ptr<U2SequenceObject> seqObj(NULL);
    U2SequenceObject *dna = NULL;
    QString annotationName;

    if (data.contains(BaseSlots::DNA_SEQUENCE_SLOT().getId())) {
        SharedDbiDataHandler seqId = data[BaseSlots::DNA_SEQUENCE_SLOT().getId()].value<SharedDbiDataHandler>();
        seqObj.reset(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));
        SAFE_POINT(NULL != seqObj.get(), tr("Genbank writer: NULL sequence object"), );

        DNASequence seq = seqObj->getWholeSequence();
        QMapIterator<QString, QVariant> it(seq.info);
        while (it.hasNext()) {
            it.next();
            if ( !(it.value().type() == QVariant::String || it.value().type() == QVariant::StringList) ) {
                seq.info.remove(it.key());
            }
        }

        if (seq.getName().isEmpty()) {
            int num = doc->findGObjectByType(GObjectTypes::SEQUENCE).size();
            seq.setName(QString("unknown sequence %1").arg(num));
        } else {
            annotationName = getAnnotationName(seq.getName());
        }

        dna = qobject_cast<U2SequenceObject*>(doc->findGObjectByName(seq.getName()));
        if (!dna && !seq.isNull()) {
            dna = addSeqObject(doc, seq);
        }
    }

    if (data.contains(BaseSlots::ANNOTATION_TABLE_SLOT().getId())) {
        const QVariant &annsVar = data[BaseSlots::ANNOTATION_TABLE_SLOT().getId()];
        QList<SharedAnnotationData> atl = QVariantUtils::var2ftl(annsVar.toList());

        if (!atl.isEmpty()) {
            AnnotationTableObject *att = NULL;
            if (dna) {
                QList<GObject*> relAnns = GObjectUtils::findObjectsRelatedToObjectByRole(dna, GObjectTypes::ANNOTATION_TABLE, GObjectRelationRole::SEQUENCE, doc->getObjects(), UOF_LoadedOnly);
                att = relAnns.isEmpty() ? NULL : qobject_cast<AnnotationTableObject*>(relAnns.first());
            }
            if (!att) {
                if (annotationName.isEmpty()) {
                    int featuresNum = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE).size();
                    annotationName = QString("unknown features %1").arg(featuresNum);
                }
                att = qobject_cast<AnnotationTableObject*>(doc->findGObjectByName(annotationName));
                if (NULL == att) {
                    doc->addObject(att = new AnnotationTableObject(annotationName));
                    if (dna) {
                        att->addObjectRelation(dna, GObjectRelationRole::SEQUENCE);
                    }
                }
                algoLog.trace(QString("Adding features [%1] to GB doc %2").arg(annotationName).arg(doc->getURLString()));
            }
            foreach(SharedAnnotationData sad, atl) {
                att->addAnnotation(new Annotation(sad), QString());
            }
        }
    }
}

bool GenbankWriter::hasDataToWrite(const QVariantMap &data) const {
    return SeqWriter::hasSequenceOrAnns(data);
}

void GenbankWriter::streamingStoreEntry(DocumentFormat* format, IOAdapter *io, const QVariantMap &data, WorkflowContext *context, int entryNum) {
    U2OpStatus2Log os;
    std::auto_ptr<U2SequenceObject> seqObj(NULL);
    QString annotationName;
    if (data.contains(BaseSlots::DNA_SEQUENCE_SLOT().getId())) {
        seqObj.reset(getCopiedSequenceObject(data, context, os));
        SAFE_POINT_OP(os, );

        if (seqObj->getGObjectName().isEmpty()) {
            seqObj->setGObjectName(QString("unknown sequence %1").arg(entryNum));
            annotationName = QString("unknown features %1").arg(entryNum);
        } else {
            annotationName = getAnnotationName(seqObj->getGObjectName());
        }
    }

    QList<GObject*> anObjList;
    if (data.contains(BaseSlots::ANNOTATION_TABLE_SLOT().getId())) {
        const QVariant &annsVar = data[BaseSlots::ANNOTATION_TABLE_SLOT().getId()];
        QList<SharedAnnotationData> atl = QVariantUtils::var2ftl(annsVar.toList());
        if (!atl.isEmpty()) {
            if (annotationName.isEmpty()) {
                annotationName = QString("unknown features %1").arg(entryNum);
            }
            AnnotationTableObject* att = new AnnotationTableObject(annotationName);
            anObjList << att;
            foreach(SharedAnnotationData sad, atl) {
                att->addAnnotation(new Annotation(sad), QString());
            }
        }
    }

    QMap< GObjectType, QList<GObject*> > objectsMap;
    {
        if (NULL != seqObj.get()) {
            QList<GObject*> seqs; seqs << seqObj.get();
            objectsMap[GObjectTypes::SEQUENCE] = seqs;
        }
        if (!anObjList.isEmpty()) {
            objectsMap[GObjectTypes::ANNOTATION_TABLE] = anObjList;
        }
    }
    format->storeEntry(io, objectsMap, os);

    foreach (GObject *o, anObjList) {
        delete o;
    }
}

/*************************************
 * GFFWriter
 *************************************/
void GFFWriter::data2doc(Document* doc, const QVariantMap& data) {
    data2document(doc, data, context);
}

bool GFFWriter::hasDataToWrite(const QVariantMap &data) const {
    return SeqWriter::hasSequenceOrAnns(data);
}

void GFFWriter::data2document(Document* doc, const QVariantMap& data, WorkflowContext *context) {
    std::auto_ptr<U2SequenceObject> seqObj(NULL);
    U2SequenceObject *dna = NULL;
    QString annotationName;
    if (data.contains(BaseSlots::DNA_SEQUENCE_SLOT().getId())) {
        SharedDbiDataHandler seqId = data[BaseSlots::DNA_SEQUENCE_SLOT().getId()].value<SharedDbiDataHandler>();
        seqObj.reset(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));
        SAFE_POINT(NULL != seqObj.get(), tr("GFF writer: NULL sequence object"), );

        DNASequence seq = seqObj->getWholeSequence();
        if (seq.getName().isEmpty()) {
            int num = doc->findGObjectByType(GObjectTypes::SEQUENCE).size();
            seq.setName(QString("unknown sequence %1").arg(num));
        } else {
            annotationName = getAnnotationName(seq.getName());
        }

        dna = qobject_cast<U2SequenceObject*>(doc->findGObjectByName(seq.getName()));
        if (!dna && !seq.isNull()) {
            dna = addSeqObject(doc, seq);
        }
    }

    if (data.contains(BaseSlots::ANNOTATION_TABLE_SLOT().getId())) {
        const QVariant &annsVar = data[BaseSlots::ANNOTATION_TABLE_SLOT().getId()];
        QList<SharedAnnotationData> atl = QVariantUtils::var2ftl(annsVar.toList());
        if (!atl.isEmpty()) {
            AnnotationTableObject* att = NULL;
            if (dna) {
                QList<GObject*> relAnns = GObjectUtils::findObjectsRelatedToObjectByRole(dna, GObjectTypes::ANNOTATION_TABLE, GObjectRelationRole::SEQUENCE, doc->getObjects(), UOF_LoadedOnly);
                att = relAnns.isEmpty() ? NULL : qobject_cast<AnnotationTableObject*>(relAnns.first());
            }
            if (!att) {
                if (annotationName.isEmpty()) {
                    int featuresNum = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE).size();
                    annotationName = QString("unknown features %1").arg(featuresNum);
                }
                att = qobject_cast<AnnotationTableObject*>(doc->findGObjectByName(annotationName));
                if (NULL == att) {
                    doc->addObject(att = new AnnotationTableObject(annotationName));
                    if (dna) {
                        att->addObjectRelation(dna, GObjectRelationRole::SEQUENCE);
                    }
                }
                algoLog.trace(QString("Adding features [%1] to GFF doc %2").arg(annotationName).arg(doc->getURLString()));
            }
            foreach(SharedAnnotationData sad, atl) {
                att->addAnnotation(new Annotation(sad), QString());
            }
        }
    }
}


/*************************************
* SeqWriter
*************************************/
SeqWriter::SeqWriter(Actor *a)
: BaseDocWriter(a), numSplitSequences(1), currentSplitSequence(0)
{

}

SeqWriter::SeqWriter(Actor *a, const DocumentFormatId &fid)
: BaseDocWriter(a, fid), numSplitSequences(1), currentSplitSequence(0)
{

}

void SeqWriter::data2doc(Document* doc, const QVariantMap& data){
    if (NULL == format) {
        return;
    }
    DocumentFormatId fid = format->getFormatId();
    if (BaseDocumentFormats::FASTA == fid) {
        FastaWriter::data2document(doc, data, context, numSplitSequences, currentSplitSequence);
        currentSplitSequence++;
    } else if (BaseDocumentFormats::PLAIN_GENBANK == fid) {
        GenbankWriter::data2document(doc, data, context);
    } else if (BaseDocumentFormats::FASTQ == fid) {
        FastQWriter::data2document(doc, data, context);
    } else if (BaseDocumentFormats::RAW_DNA_SEQUENCE == fid) {
        RawSeqWriter::data2document(doc, data, context);
    } else if (BaseDocumentFormats::GFF == fid) {
        GFFWriter::data2document(doc, data, context);
    } else {
        assert(0);
        ioLog.error(QString("Unknown data format for writing: %1").arg(fid));
    }
}

bool SeqWriter::hasSequence(const QVariantMap &data) {
    return data.contains(BaseSlots::DNA_SEQUENCE_SLOT().getId());
}

bool SeqWriter::hasSequenceOrAnns(const QVariantMap &data) {
    return data.contains(BaseSlots::DNA_SEQUENCE_SLOT().getId())
        || data.contains(BaseSlots::ANNOTATION_TABLE_SLOT().getId());
}

bool SeqWriter::hasDataToWrite(const QVariantMap &data) const {
    CHECK(NULL != format, false);
    
    DocumentFormatId fid = format->getFormatId();
    if (BaseDocumentFormats::GFF == fid
        || BaseDocumentFormats::PLAIN_GENBANK == fid) {
        return hasSequenceOrAnns(data);
    } else {
        return hasSequence(data);
    }
}

void SeqWriter::storeEntry(IOAdapter *io, const QVariantMap &data, int entryNum) {
    if( format == NULL ) {
        return;
    }
    DocumentFormatId fid = format->getFormatId();
    if( fid == BaseDocumentFormats::FASTA ) {
        FastaWriter::streamingStoreEntry(format, io, data, context, entryNum);
        currentSplitSequence++;
    } else if( fid == BaseDocumentFormats::PLAIN_GENBANK ) {
        GenbankWriter::streamingStoreEntry(format, io, data, context, entryNum);
    } else if ( fid == BaseDocumentFormats::FASTQ) {
        FastQWriter::streamingStoreEntry(format, io, data, context, entryNum);
    } else if( fid == BaseDocumentFormats::RAW_DNA_SEQUENCE ) {
        RawSeqWriter::streamingStoreEntry(format, io, data, context, entryNum);
    } else {
        assert(0);
        ioLog.error(QString("Unknown data format for writing: %1").arg(fid));
    }
}

void SeqWriter::takeParameters(U2OpStatus &os) {
    BaseDocWriter::takeParameters(os);
    SAFE_POINT_OP( os, );

    Attribute *splitAttr = actor->getParameter(BaseAttributes::SPLIT_SEQ_ATTRIBUTE().getId());
    if (format->getFormatId() == BaseDocumentFormats::FASTA && splitAttr != NULL) {
        numSplitSequences = splitAttr->getAttributeValue<int>(context);
    } else{
        numSplitSequences = 1;
    }
}

QStringList SeqWriter::takeUrlList(const QVariantMap &data, U2OpStatus &os) {
    QStringList urls = BaseDocWriter::takeUrlList(data, os);
    SAFE_POINT_OP(os, urls);
    SAFE_POINT(1 == urls.size(), "Several urls in the output attribute", urls);

    SharedDbiDataHandler seqId = data.value(BaseSlots::DNA_SEQUENCE_SLOT().getId()).value<SharedDbiDataHandler>();
    QSharedPointer<U2SequenceObject> seqObj(StorageUtils::getSequenceObject(context->getDataStorage(), seqId));

    currentSplitSequence = 0;

    if (!seqObj.data()) {
        numSplitSequences = 1;
    } else {
        qint64 seqLen = seqObj.data()->getSequenceLength();
        if (seqLen < numSplitSequences) {
            numSplitSequences = seqLen;
        }
        if (0 == numSplitSequences) {
            numSplitSequences = 1;
        }
    }

    if (numSplitSequences > 1) {
        QString url = urls.takeFirst();
        for (int i = 0; i < numSplitSequences; i++) {
            urls << GUrlUtils::insertSuffix(url, "_split" + QString::number(i+1));
        }
    }

    return urls;
}

bool SeqWriter::isStreamingSupport() const {
    if (numSplitSequences > 1) {
        return false;
    }
    return BaseDocWriter::isStreamingSupport();
}

/*************************************
* MSAWriter
*************************************/
void MSAWriter::data2doc(Document* doc, const QVariantMap& data) {
    data2document(doc, data, context);
}

void MSAWriter::data2document(Document* doc, const QVariantMap& data, WorkflowContext* context) {
    SharedDbiDataHandler msaId = data.value(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId()).value<SharedDbiDataHandler>();
    std::auto_ptr<MAlignmentObject> msaObj(StorageUtils::getMsaObject(context->getDataStorage(), msaId));
    SAFE_POINT(NULL != msaObj.get(), "NULL MSA Object!", );
    MAlignment ma = msaObj->getMAlignment();

    SAFE_POINT(!ma.isEmpty(), tr("Empty alignment passed for writing to %1").arg(doc->getURLString()), )

    if (ma.getName().isEmpty()) {
        QString name = QString(MA_OBJECT_NAME + "_%1").arg(ct);
        ma.setName(name);
        ct++;
    }

    U2OpStatus2Log os;
    U2EntityRef msaRef = MAlignmentImporter::createAlignment(doc->getDbiRef(), ma, os);
    CHECK_OP(os, );

    MAlignmentObject* obj = new MAlignmentObject(ma.getName(), msaRef);
    doc->addObject(obj);
}

bool MSAWriter::hasDataToWrite(const QVariantMap &data) const {
    return data.contains(BaseSlots::MULTIPLE_ALIGNMENT_SLOT().getId());
}

bool MSAWriter::isStreamingSupport() const {
    return false;
}

/*************************************
* DataWorkerFactory
*************************************/
Worker* DataWorkerFactory::createWorker(Actor* a) {
    // TODO: wtf is this??
    //  each actor must have own factory 

    BaseWorker* w = NULL;
    QString protoId = a->getProto()->getId();
    if (CoreLibConstants::READ_TEXT_PROTO_ID == protoId ) {
        TextReader* t = new TextReader(a);
        w = t;
    } 
    else if (CoreLibConstants::WRITE_TEXT_PROTO_ID == protoId) {
        w = new TextWriter(a);
    } 
    else if (CoreLibConstants::WRITE_FASTA_PROTO_ID == protoId) {
        w = new FastaWriter(a);
    }
    else if (CoreLibConstants::WRITE_GENBANK_PROTO_ID == protoId) {
        w = new GenbankWriter(a);
    }
    else if (CoreLibConstants::WRITE_CLUSTAL_PROTO_ID == protoId) {
        w = new MSAWriter(a, BaseDocumentFormats::CLUSTAL_ALN);
    }
    else if (CoreLibConstants::WRITE_STOCKHOLM_PROTO_ID == protoId) {
        w = new MSAWriter(a, BaseDocumentFormats::STOCKHOLM);
    }
    else if (CoreLibConstants::GENERIC_READ_MA_PROTO_ID == protoId) {
        w = new GenericMSAReader(a);
    }
    else if (CoreLibConstants::GENERIC_READ_SEQ_PROTO_ID == protoId) {
        w = new GenericSeqReader(a);
    } 
    else if(CoreLibConstants::WRITE_MSA_PROTO_ID == protoId ) {
        w = new MSAWriter(a);
    }
    else if(CoreLibConstants::WRITE_SEQ_PROTO_ID == protoId ) {
        w = new SeqWriter(a);
    } 
    else if (CoreLibConstants::WRITE_FASTQ_PROTO_ID == protoId ) {
        w = new FastQWriter(a);
    } else {
        assert(0);
    }
    return w;    
}

void DataWorkerFactory::init() {
    DomainFactory* localDomain = WorkflowEnv::getDomainRegistry()->getById(LocalDomainFactory::ID);
    localDomain->registerEntry(new DataWorkerFactory(CoreLibConstants::WRITE_FASTA_PROTO_ID));
    localDomain->registerEntry(new DataWorkerFactory(CoreLibConstants::WRITE_GENBANK_PROTO_ID));
    localDomain->registerEntry(new DataWorkerFactory(CoreLibConstants::READ_TEXT_PROTO_ID));
    localDomain->registerEntry(new DataWorkerFactory(CoreLibConstants::WRITE_TEXT_PROTO_ID));
    localDomain->registerEntry(new DataWorkerFactory(CoreLibConstants::GENERIC_READ_SEQ_PROTO_ID));
    localDomain->registerEntry(new DataWorkerFactory(CoreLibConstants::GENERIC_READ_MA_PROTO_ID));
    localDomain->registerEntry(new DataWorkerFactory(CoreLibConstants::WRITE_CLUSTAL_PROTO_ID));
    localDomain->registerEntry(new DataWorkerFactory(CoreLibConstants::WRITE_STOCKHOLM_PROTO_ID));
    localDomain->registerEntry(new DataWorkerFactory(CoreLibConstants::WRITE_MSA_PROTO_ID));
    localDomain->registerEntry(new DataWorkerFactory(CoreLibConstants::WRITE_SEQ_PROTO_ID));
    localDomain->registerEntry(new DataWorkerFactory(CoreLibConstants::WRITE_FASTQ_PROTO_ID));
}

} // Workflow namespace
} // U2 namespace
