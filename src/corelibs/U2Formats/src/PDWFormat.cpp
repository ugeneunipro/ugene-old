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

#include "PDWFormat.h"

#include "DocumentFormatUtils.h"

#include <U2Core/AppContext.h>
#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/L10n.h>
#include <U2Core/GObjectRelationRoles.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/TextUtils.h>

namespace U2 {

/* TRANSLATOR U2::FastaFormat */
/* TRANSLATOR U2::IOAdapter */
/* TRANSLATOR U2::Document */


#define PDW_FORMAT_TAG      "VERSION  pDRAW 1.0"
#define PDW_SEQUENCE_TAG    "Sequence .."
#define PDW_DNANAME_TAG     "DNAname"
#define PDW_ELEMENT_TAG     "Element"
#define PDW_ANNOTATION_TAG  "Annotation"
#define PDW_CIRCULAR_TAG    "IScircular"

PDWFormat::PDWFormat(QObject* p) 
: DocumentFormat(p, DocumentFormatFlag(0), QStringList()<<"pdw")
{
    formatName = tr("pDRAW");
    supportedObjectTypes+=GObjectTypes::SEQUENCE;
    supportedObjectTypes+=GObjectTypes::ANNOTATION_TABLE;
}

FormatDetectionResult PDWFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    const char* data = rawData.constData();
    int size = rawData.size();
    if (!rawData.startsWith(PDW_FORMAT_TAG)) {
        return FormatDetection_NotMatched;
    }

    bool hasBinaryBlocks = TextUtils::contains(TextUtils::BINARY, data, size);
    return hasBinaryBlocks ? FormatDetection_NotMatched : FormatDetection_HighSimilarity;
}

#define READ_BUFF_SIZE  4096
void PDWFormat::load(IOAdapter* io, const GUrl& docUrl, QList<GObject*>& objects, TaskStateInfo& ti, 
                     DNASequenceObject* dnaObj, AnnotationTableObject* aObj)
{
    
    QByteArray readBuff(READ_BUFF_SIZE+1, 0);
    char* buff = readBuff.data();
    qint64 len = 0;
    
    bool lineOk = false;
    bool isCircular = false;
    QString dnaName(docUrl.baseFileName());
    QList<Annotation*> annotations;
    
    while (!ti.cancelFlag) {
        //read header
        len = io->readUntil(buff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
        if (len == 0) { //end if stream
            break;
        }
        if (!lineOk) {
            ti.setError(PDWFormat::tr("Line is too long"));
        }
        
        if (readBuff.startsWith(PDW_DNANAME_TAG)) {
            dnaName = readPdwValue(readBuff, PDW_DNANAME_TAG);
        } else if (readBuff.startsWith(PDW_SEQUENCE_TAG)) {
            QByteArray seq = parseSequence(io, ti);
            DNAAlphabet* alphabet = AppContext::getDNAAlphabetRegistry()->findAlphabet(seq);
            DNASequence dna(dnaName, seq , alphabet);
            if (isCircular) {
                DNALocusInfo loi;
                loi.topology = "circular";
                loi.name = dnaName;
                dna.info.insert(DNAInfo::LOCUS, qVariantFromValue<DNALocusInfo>(loi));
            }
            dnaObj = new DNASequenceObject(dnaName, dna);
            objects.append(dnaObj);
            break;
        } else if (readBuff.startsWith(PDW_CIRCULAR_TAG)) {
            QByteArray val = readPdwValue(readBuff, PDW_CIRCULAR_TAG);
            if (val == "YES") {
                isCircular = true;
            }
        } else if (readBuff.startsWith(PDW_ANNOTATION_TAG)) {
            Annotation* a = parseAnnotation(io, ti);
            assert(a != NULL);
            annotations.append(a);
        }  
    }
    
    if (!annotations.isEmpty()) {
        aObj = new AnnotationTableObject(QString("%1 annotations").arg(dnaName));
        aObj->addAnnotations(annotations);
        objects.append(aObj);

    }

    if (!ti.hasError() && !ti.cancelFlag && objects.isEmpty()) {
        ti.setError(Document::tr("Document is empty."));
    }
}


Document* PDWFormat::loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& _fs, DocumentLoadMode mode) {
    Q_UNUSED(mode);
  
    DNASequenceObject* dnaObj = NULL;
    AnnotationTableObject* aObj = NULL;
    
    if( NULL == io || !io->isOpen() ) {
        ti.setError(L10N::badArgument("IO adapter"));
        return NULL;
    }
    
    QVariantMap fs = _fs;
    QList<GObject*> objects;
    
    load(io, io->getURL(), objects, ti, dnaObj, aObj);

    if (ti.hasError() || ti.cancelFlag) {
        qDeleteAll(objects);
        return NULL;
    }
    
    QString lockReason(DocumentFormat::CREATED_NOT_BY_UGENE);
    Document* doc = new Document(this, io->getFactory(), io->getURL(), objects, fs, lockReason);

    if ( (dnaObj != NULL) && (aObj != NULL)) {
        aObj->addObjectRelation(dnaObj, GObjectRelationRole::SEQUENCE);
    }

    return doc;
}

QByteArray PDWFormat::parseSequence( IOAdapter* io, TaskStateInfo& ti )
{
    QByteArray result;
    
    QByteArray readBuff(READ_BUFF_SIZE+1, 0);
    
    while (!ti.cancelFlag) {
    
        bool lineOk = false;
        qint64 len = io->readUntil(readBuff.data(), READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
        if (len == 0) { 
            break;
        }
        if (!lineOk) {
            ti.setError(PDWFormat::tr("Line is too long"));
        } 
        
        for (int i = 0; i < readBuff.size(); ++i) {
            char c = readBuff.at(i);
            if (c == '\n') {
                break;
            }
            if (c >= 'A' && c <= 'z') {
                result.append(c);
            }
            

        }
    }

    return result;


}

QByteArray PDWFormat::readPdwValue( const QByteArray& readBuf, const QByteArray& valueName )
{
    int startPos = valueName.length();
    int endPos = readBuf.indexOf('\n');
    return readBuf.mid(startPos, endPos - startPos + 1).trimmed();
}

#define PDW_ANNOTATION_NUMBER   "Annotation_Number"
#define PDW_ANNOTATION_NAME     "Annotation_Name"
#define PDW_ANNOTATION_START    "Annotation_Start"
#define PDW_ANNOTATION_END      "Annotation_End"
#define PDW_ANNOTATION_ORIENT   "Annotation_Orientation"

Annotation* PDWFormat::parseAnnotation( IOAdapter* io, TaskStateInfo& ti )
{
    QByteArray readBuf(READ_BUFF_SIZE+1, 0);

    int startPos = -1, endPos = -1;
    QByteArray aName;
    bool cmpl = false;

    while (!ti.cancelFlag) {

        bool lineOk = false;
        qint64 len = io->readUntil(readBuf.data(), READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
        
        if (!readBuf.startsWith(PDW_ANNOTATION_TAG)) {
            break;
        } else if (readBuf.startsWith(PDW_ANNOTATION_NUMBER)) {
            io->skip(-len);
            break;
        } else if (readBuf.startsWith(PDW_ANNOTATION_NAME) ) {
            aName = readPdwValue(readBuf, PDW_ANNOTATION_NAME);
            if (aName.endsWith('\"')) {
                aName = aName.left(aName.length() - 1).trimmed(); 
            }
        } else if (readBuf.startsWith(PDW_ANNOTATION_START) ) {
            startPos = readPdwValue(readBuf, PDW_ANNOTATION_START).toInt();
        } else if (readBuf.startsWith(PDW_ANNOTATION_END) ) {
            endPos = readPdwValue(readBuf, PDW_ANNOTATION_END).toInt();
        }else if (readBuf.startsWith(PDW_ANNOTATION_ORIENT) ) {
            int orientVal = readPdwValue(readBuf, PDW_ANNOTATION_ORIENT).toInt();
            cmpl = orientVal == 0 ? true : false;
        }
    
    }
    
    SharedAnnotationData sd(new AnnotationData()); 
    
    sd->name = aName;
    sd->location->regions << U2Region(startPos - 1, endPos - startPos + 1);
    sd->setStrand(cmpl ? U2Strand::Complementary : U2Strand::Direct);

    return new Annotation(sd);

}




}//namespace
