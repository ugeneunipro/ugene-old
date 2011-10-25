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


#include "EMBLGenbankAbstractDocument.h"

#include "GenbankLocationParser.h"
#include "GenbankFeatures.h"
#include "DocumentFormatUtils.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectRelationRoles.h>

#include <U2Core/IOAdapter.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/L10n.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/DNAInfo.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U1AnnotationUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2SequenceDbi.h>

#include <memory>

namespace U2 {

/* TRANSLATOR U2::EMBLGenbankAbstractDocument */    
//TODO: local8bit or ascii??

EMBLGenbankAbstractDocument::EMBLGenbankAbstractDocument(const DocumentFormatId& _id, const QString& _formatName, int mls, 
                                                         DocumentFormatFlags flags, QObject* p) 
: DocumentFormat(p, flags), id(_id), formatName(_formatName), maxAnnotationLineLen(mls)
{
    supportedObjectTypes+=GObjectTypes::ANNOTATION_TABLE;
    supportedObjectTypes+=GObjectTypes::SEQUENCE;
}


//////////////////////////////////////////////////////////////////////////
// loading

Document* EMBLGenbankAbstractDocument::loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& _fs, U2OpStatus& os) {
    QVariantMap fs = _fs;
    QList<GObject*> objects;
    QString writeLockReason;
    load(dbiRef, io, objects, fs, os, writeLockReason);

    CHECK_OP(os, NULL);
    
    DocumentFormatUtils::updateFormatHints(objects, fs);
    Document* doc = new Document(this, io->getFactory(), io->getURL(), dbiRef, dbiRef.isValid(), objects, fs, writeLockReason);
    return doc;
}



const QString EMBLGenbankAbstractDocument::UGENE_MARK("UNIMARK");
const QString EMBLGenbankAbstractDocument::DEFAULT_OBJ_NAME("unnamed");


void EMBLGenbankAbstractDocument::load(const U2DbiRef& dbiRef, IOAdapter* io, QList<GObject*>& objects, QVariantMap& fs, U2OpStatus& os, QString& writeLockReason) {
    writeLockReason.clear();

    //get settings
    int gapSize = qBound(-1, DocumentFormatUtils::getMergeGap(fs), 1000*1000);
    bool merge = gapSize!=-1;

    QByteArray  gapSequence((merge ? gapSize : 0), 0);
    AnnotationTableObject* mergedAnnotations = NULL;
    QStringList contigs;
	QVector<U2Region> mergedMapping;
	U2SequenceImporter seqImporter;
    
    QSet<QString> usedNames;
    bool toolMark = false;
    
    GObjectReference sequenceRef(GObjectReference(io->getURL().getURLString(), "", GObjectTypes::SEQUENCE));

    QByteArray readBuffer(ParserState::READ_BUFF_SIZE, '\0');
    ParserState st(getFormatId() == BaseDocumentFormats::PLAIN_GENBANK ? 12 : 5, io, NULL, os);
    st.buff = readBuffer.data();

    TmpDbiObjects dbiObjects(dbiRef, os);
	int num_sequence = 0;

	int sequenceStart = 0;
	int sequenceSize = 0;
	int fullSequenceSize = 0;

    for (int i=0; !os.isCoR(); i++, ++num_sequence) {	
		EMBLGenbankDataEntry data;
		st.entry = &data;	

		if (num_sequence == 0 || merge == false){
			seqImporter.startSequence(dbiRef,"",false,os); //change name and circularity after finalize method
			CHECK_OP(os,);
		}		

		sequenceSize = 0;
        os.setDescription(tr("Reading entry header"));
		if (!readEntry(&st,seqImporter,sequenceSize,fullSequenceSize,merge, (num_sequence > 0) ? gapSize : 0, os)) {
			break;
		}

		if (merge && sequenceSize > 0 && num_sequence > 0) {
				sequenceStart += sequenceSize ;
				sequenceStart += gapSize;
				fullSequenceSize += gapSize;
		}

		// tolerate blank lines between records
        char ch;
        bool b;
        while ((b = st.io->getChar(&ch)) && (ch == '\n' || ch == '\r')){}
        if (b) {
            st.io->skip(-1);
        }

		toolMark = data.tags.contains(UGENE_MARK);
		AnnotationTableObject* annotationsObject  = NULL;

		if (data.hasAnnotationObjectFlag) {
			QString annotationName = genObjectName(usedNames, data.name, data.tags, i+1, GObjectTypes::ANNOTATION_TABLE);
			if (merge && mergedAnnotations == NULL) {
				mergedAnnotations = new AnnotationTableObject(annotationName);
			}
			annotationsObject = merge ? mergedAnnotations : new AnnotationTableObject(annotationName);

			QStringList groupNames;
			foreach(SharedAnnotationData d, data.features) {
				groupNames.clear();
				d->removeAllQualifiers(GBFeatureUtils::QUALIFIER_GROUP, groupNames);
				if (groupNames.isEmpty()) {
					annotationsObject->addAnnotation(new Annotation(d));
				} else {
					Annotation* a = new Annotation(d);
					foreach(const QString& gName, groupNames) {
						annotationsObject->getRootGroup()->getSubgroup(gName, true)->addAnnotation(a);
					}
				}
			}

			if (!merge) {
				objects.append(annotationsObject);
			}
		} else{
			assert(data.features.isEmpty());
		}
        
        if (!os.isCoR()) {
			QString sequenceName = genObjectName(usedNames, data.name, data.tags, i+1, GObjectTypes::SEQUENCE);
            if (merge && sequenceSize == 0 && annotationsObject!=NULL) {
                os.setError(tr("Merge error: found annotations without sequence"));
                break;
            } 
			else if (merge) {
		        contigs.append(sequenceName);
                mergedMapping.append(U2Region(sequenceStart, sequenceSize));
			} 
			else { 
				U2Sequence u2seq = seqImporter.finalizeSequence(os);
				CHECK_OP(os, );
				u2seq.visualName = sequenceName;
				u2seq.circular = data.circular;
				DbiConnection con(dbiRef, os);				
				con.dbi->getSequenceDbi()->updateSequenceObject(u2seq,os);

				if(sequenceSize != 0){
					fullSequenceSize = 0;
					
					U2SequenceObject* seqObj =  new U2SequenceObject(sequenceName, U2EntityRef(dbiRef, u2seq.id));
					objects << seqObj;
		
				    SAFE_POINT(seqObj != NULL, "DocumentFormatUtils::addSequenceObject returned NULL but didn't set error",);
				    dbiObjects.objects << seqObj->getSequenceRef().entityId;

					if (annotationsObject!=NULL) {				
						sequenceRef.objName = seqObj->getGObjectName();
						annotationsObject->addObjectRelation(GObjectRelation(sequenceRef, GObjectRelationRole::SEQUENCE));					
					}
				}
			}
		}
    }
    CHECK_OP(os, );
    CHECK_EXT(!objects.isEmpty() || merge, os.setError(Document::tr("Document is empty.")), );
    SAFE_POINT(contigs.size() == mergedMapping.size(), "contigs <-> regions mapping failed!", );

    if (!toolMark) {
        writeLockReason = DocumentFormat::CREATED_NOT_BY_UGENE;
    } else if (merge) {
        writeLockReason = DocumentFormat::MERGED_SEQ_LOCK;
    }
	
    if (!merge) {
        return;
    }
	U2Sequence u2seq = seqImporter.finalizeSequence(os);
	CHECK_OP(os,);

	u2seq.visualName = "Sequence";
	DbiConnection con(dbiRef, os);
	con.dbi->getSequenceDbi()->updateSequenceObject(u2seq,os);

    if (os.hasError()) {
        qDeleteAll(objects);
        delete mergedAnnotations;
        return;
    }
	U2SequenceObject* so = new U2SequenceObject(u2seq.visualName, U2EntityRef(dbiRef, u2seq.id));
	objects << so;
	objects << DocumentFormatUtils::addAnnotationsForMergedU2Sequence(io->getURL(), contigs, u2seq, mergedMapping, os);
    if (mergedAnnotations!=NULL) {
        sequenceRef.objName = so->getGObjectName();
        mergedAnnotations->addObjectRelation(GObjectRelation(sequenceRef, GObjectRelationRole::SEQUENCE));
        objects.append(mergedAnnotations);
    }

}

//column annotation data starts with
#define A_COL 21
//column qualifier name starts with
#define QN_COL 22
//column annotation key starts with
#define K_COL 5


static bool isNewQStart(const char* s, int l) {
    if (l < A_COL + 1 || s[A_COL]!='/') {
        return false;
    }
    const QBitArray& WHITES = TextUtils::WHITES;
    for (int i = QN_COL; i < l; i++) {
        char c = s[i];
        if (c == '=' && i > QN_COL) {
            return true;
        }
        if (WHITES[(uchar)c]) {
            break;
        }
    }
    return false;
}

//TODO: make it IO active -> read util the end. Otherwise qualifier is limited in size by maxSize
int EMBLGenbankAbstractDocument::readMultilineQualifier(IOAdapter* io, char* cbuff, int maxSize, bool _prevLineHasMaxSize) {
    int len = 0;
    bool lineOk = true;
    static const int MAX_LINE = 256;
    int sizeToSkip = maxSize - MAX_LINE;
    const QBitArray& LINE_BREAKS = TextUtils::LINE_BREAKS;
    bool breakWords = !_prevLineHasMaxSize; //todo: create a parameter and make it depends on annotation name.
    do {
        if (len >= sizeToSkip) {
            QByteArray skip(MAX_LINE, 0);
            char* skipBuff = skip.data();
            do {
                int readLen = io->readUntil(skipBuff, MAX_LINE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
                if (!lineOk) {
                    continue;//todo: report error!
                }
                int lineLen = readLen;
                for (; A_COL < lineLen && LINE_BREAKS[(uchar)skipBuff[lineLen-1]]; lineLen--){}; //remove line breaks
                if (lineLen == 0 || lineLen < A_COL || skip[0]!=fPrefix[0] || skip[1]!=fPrefix[1] 
                    || skip[K_COL]!=' ' || (skip[A_COL]=='/' && isNewQStart(skip, lineLen))) {
                    io->skip(-readLen);
                    break;
                }
            } while (true);
            break;
        }
        char* lineBuf = cbuff + len;
        int readLen = io->readUntil(lineBuf, maxSize-len, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk);
        int lineLen = readLen;
        for (; A_COL < lineLen && LINE_BREAKS[(uchar)lineBuf[lineLen-1]]; lineLen--){}; //remove line breaks
        if (!lineOk || lineLen == 0 || lineLen < A_COL || lineBuf[0]!=fPrefix[0]  
            || lineBuf[1]!=fPrefix[1] || lineBuf[K_COL]!=' ' || (lineBuf[A_COL]=='/' && isNewQStart(lineBuf, lineLen))) 
        {
            io->skip(-readLen);
            break;
        }
        if (breakWords && lineLen-A_COL > 0 && lineBuf[A_COL]!=' ') { //add space to separate words
            cbuff[len] = ' ';
            len++;
        }
        qMemCopy(cbuff + len, lineBuf + A_COL, lineLen - A_COL);
        len+=lineLen-A_COL;
        breakWords = breakWords || lineLen < maxAnnotationLineLen;
    } while (true);
    return len;
}


QString EMBLGenbankAbstractDocument::genObjectName(QSet<QString>& usedNames, const QString& seqName, const QVariantMap& tags, int n, const GObjectType& t) {
    //try to check UGENE_MARK first
    QString name;
    QStringList unimark = tags.value(UGENE_MARK).toStringList();
    if (!unimark.isEmpty()) {
        int pos = usedNames.size();
        if (pos < unimark.size()) {
            name = unimark.at(pos);
        }
    }
    if (name.isEmpty()) {
        name = seqName;
        if (name.isEmpty()) {
            name = DNAInfo::getPrimaryAccession(tags);
            int spaceIdx = name.indexOf(' ');
            if (spaceIdx > 0) {
                name = name.left(spaceIdx-1);
            }
            if (name.isEmpty()) {
                name = EMBLGenbankAbstractDocument::DEFAULT_OBJ_NAME;
            }
        }
        if(t == GObjectTypes::ANNOTATION_TABLE) {
            name+=" features";
        } else if (t == GObjectTypes::SEQUENCE) {
            name+=" sequence";
        }
    }
    //now variate name if there are duplicates
    int n2 = 1;
    QString res = name;
    while (usedNames.contains(res)) {
        res =  name + " " + QString::number(n) + (n2 == 1 ? QString("") : ("."+QString::number(n2)));
    }
    usedNames.insert(res);
    return res;
}

static void checkQuotes(const char* str, int len, bool& outerQuotes, bool& doubleQuotes) {
    char qChar = '\"';
    assert(len>=0);
    outerQuotes = str[0] == qChar && str[len-1]==qChar;
    for(int i=1; i < len; i++) {
        if (str[i-1]==qChar && str[i] == qChar) {
            doubleQuotes = true;
            break;
        }
    }
}

SharedAnnotationData EMBLGenbankAbstractDocument::readAnnotation(IOAdapter* io, char* cbuff, int len, 
                                                                 int READ_BUFF_SIZE, U2OpStatus& si, int offset) 
{
    AnnotationData* a = new AnnotationData();
    SharedAnnotationData f(a);
    QString key = QString::fromAscii(cbuff+5, 15).trimmed();
    if (key.isEmpty()) {
        si.setError(EMBLGenbankAbstractDocument::tr("Annotation name is empty"));
        return SharedAnnotationData();
    }
    a->name = key;
    
    //qualifier starts on offset 22;
    int qlen = len + readMultilineQualifier(io, cbuff+len, READ_BUFF_SIZE - len, true);
    if (qlen < 21) {
        si.setError(EMBLGenbankAbstractDocument::tr("Error parsing location"));
        return SharedAnnotationData();
    }

    Genbank::LocationParser::parseLocation(cbuff+21, qlen-21, a->location);
    if (a->location->isEmpty()) {
        si.setError(EMBLGenbankAbstractDocument::tr("Error parsing location"));
        return SharedAnnotationData();
    }
    // omit sorting because of splitted annotations 
    /*if (a->location->isMultiRegion()) {
        qSort(a->location->regions);
    }*/
    if (offset!=0) {
        U2Region::shift(offset, a->location->regions);
    }

    const QBitArray& LINE_BREAKS = TextUtils::LINE_BREAKS;
    const QByteArray& aminoQ = GBFeatureUtils::QUALIFIER_AMINO_STRAND;
    //const QByteArray& aminoQYes = GBFeatureUtils::QUALIFIER_AMINO_STRAND_YES;
    //const QByteArray& aminoQNo = GBFeatureUtils::QUALIFIER_AMINO_STRAND_NO;
    const QByteArray& nameQ = GBFeatureUtils::QUALIFIER_NAME;
    
    //here we have valid key and location;
    //reading qualifiers
    bool lineOk = true;
    while ((len = io->readUntil(cbuff, READ_BUFF_SIZE, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &lineOk) ) > 0)  {
        if (len == 0 || len < QN_COL+1 || cbuff[K_COL]!=' ' || cbuff[A_COL]!='/' || cbuff[0]!=fPrefix[0] || cbuff[1]!=fPrefix[1]) {
            io->skip(-len);
            break;
        }
        if (!lineOk) {
            si.setError(EMBLGenbankAbstractDocument::tr("Unexpected line format"));
            break;
        }
        for (; QN_COL < len && LINE_BREAKS[(uchar)cbuff[len-1]]; len--){}; //remove line breaks
        int flen = len + readMultilineQualifier(io, cbuff+len, READ_BUFF_SIZE-len, len == maxAnnotationLineLen);
        //now the whole feature is in cbuff
        int valStart = A_COL + 1;
        for (; valStart < flen && cbuff[valStart] != '='; valStart++){}; //find '==' and valStart
        if (valStart < flen) {
            valStart++; //skip '=' char
        }
        const QBitArray& WHITE_SPACES = TextUtils::WHITES;
        for (; valStart < flen && WHITE_SPACES[(uchar)cbuff[flen-1]]; flen--){}; //trim value
        const char* qname = cbuff + QN_COL;
        int qnameLen = valStart - (QN_COL + 1);
        const char* qval = cbuff + valStart;
        int qvalLen = flen - valStart;
        bool removeQuotes = false;
        bool containsDoubleQuotes = false;
        checkQuotes(qval, qvalLen, removeQuotes, containsDoubleQuotes);
        if (removeQuotes) {
            qval++;
            qvalLen-=2;
        }
        if (qnameLen == aminoQ.length() && TextUtils::equals(qname, aminoQ.constData(), qnameLen)) {
            //a->aminoFrame = qvalLen == aminoQYes.length() && TextUtils::equals(qval, aminoQYes.constData(), qvalLen) ? TriState_Yes
            //             :  (qvalLen == aminoQNo.length()  && TextUtils::equals(qval, aminoQNo.constData(), qvalLen) ? TriState_No : TriState_Unknown);
        } else if (qnameLen == nameQ.length() && TextUtils::equals(qname, nameQ.constData(), qnameLen)) {
            a->name = QString::fromLocal8Bit(qval, qvalLen);
        } else {
            QString nameQStr = QString::fromLocal8Bit(qname, qnameLen);
            QString valQStr = QString::fromLocal8Bit(qval, qvalLen);
            if (containsDoubleQuotes) {
                valQStr = valQStr.replace("\"\"", "\"");
            }
            a->qualifiers.append(U2Qualifier(nameQStr, valQStr));
        }
    }
    return f;
}

bool EMBLGenbankAbstractDocument::readSequence(ParserState* st, U2SequenceImporter& seqImporter, int& sequenceLen,int& fullSequenceLen,U2OpStatus& os) {
    // FIXME use ParserState instead
	QByteArray res;
    IOAdapter* io = st->io;
    U2OpStatus& si = st->si;
    si.setDescription(tr("Reading sequence %1").arg(st->entry->name));
    int headerSeqLen = st->entry->seqLen;
    //res.reserve(res.size() + headerSeqLen);

    static int READ_BUFF_SIZE = 4096;
    QByteArray readBuffer(READ_BUFF_SIZE, '\0');
    char* buff  = readBuffer.data();

    //reading sequence      
    QBuffer writer(&res);
    writer.open( QIODevice::WriteOnly);
    bool ok = true;
    int len;
    int dataOffset = 0;
    bool numIsPrefix = getFormatId() == BaseDocumentFormats::PLAIN_GENBANK;
    while (ok && (len = io->readLine(buff, READ_BUFF_SIZE)) > 0) {
        if (si.isCoR()) {
            res.clear();
            break;
        }
        
        if (len <= 0)  {
            si.setError(tr("Error parsing sequence: unexpected empty line"));
            break;
        }

        if (buff[0] == '/') { //end of the sequence
            break;
        }
        
        //compute data offset
        bool foundNum = false;
        bool foundSpaceAfterNum = false;
        for(dataOffset = 0 ; dataOffset < len; dataOffset++) {
            char c = numIsPrefix ? buff[dataOffset] : buff[len - dataOffset - 1];
            bool isNum = c >= '0' && c <= '9';
            bool isSpace = c == ' ' || c == '\t';
            if (!isSpace && (!isNum || foundSpaceAfterNum)) {
                if (!foundSpaceAfterNum) {
                    //unknown character -> stop iteration
                    dataOffset = len;   
                }
                break;
            }
            foundNum = foundNum || isNum;
            foundSpaceAfterNum = foundSpaceAfterNum || (isSpace && foundNum);
        }
        
        if (dataOffset == len) {
            si.setError(tr("Error reading sequence: invalid sequence format"));    
            break;
        }

		bool isSeek = writer.seek(0);
		assert(isSeek);

        //add buffer to result	
        for (int i= (numIsPrefix ? dataOffset : 0), n = (numIsPrefix ? len : len -  dataOffset) ; i < n; i++) {
            char c = buff[i];
            if (c != ' ' && c != '\t') {
                ok = writer.putChar(c);
                if (!ok) {
                    break;
                }
            }
        }
        if (!ok) {
            si.setError(tr("Error reading sequence: memory allocation failed"));
            break;
        }
		
		seqImporter.addBlock(res.data(),res.size(),os);
		CHECK_OP(os,false);
		sequenceLen += res.size();
		fullSequenceLen += res.size();
		res.clear();
				
        si.setProgress(io->getProgress());
    }
    if (!si.isCoR() && buff[0] != '/') {
        si.setError(tr("Sequence is truncated"));
    }
    writer.close();
    return true; //FIXME
}

void EMBLGenbankAbstractDocument::readAnnotations(ParserState* st, int offset) {
    st->si.setDescription(tr("Reading annotations %1").arg(st->entry->name));
    st->entry->hasAnnotationObjectFlag = true;
    do {
        if (st->hasKey("XX") && getFormatId() == BaseDocumentFormats::PLAIN_EMBL) {
            continue;
        }
        int fplen = fPrefix.length();
        if (st->len >= 6 && TextUtils::equals(fPrefix.data(), st->buff, fplen)) {
            while (fplen < 5) {
                if (st->buff[fplen++] != ' ') {
                    st->si.setError(tr("Invalid format of feature table"));
                    break;
                }
            }
        } else {
            // end of feature table
            break;
        }
        //parsing feature;
        SharedAnnotationData f = readAnnotation(st->io, st->buff, st->len, ParserState::READ_BUFF_SIZE, st->si, offset);
        st->entry->features.push_back(f);
    } while (st->readNextLine());
}

bool ParserState::hasKey( const char* key, int slen ) const {
    assert(slen <= valOffset);
    if (slen <= len && TextUtils::equals(key, buff, slen)) {
        while (slen < qMin(valOffset, len)) {
            if (buff[slen++] != ' ') {
                return false;
            }
        }
        return true;
    }
    return false;
}

QString ParserState::value() const {
    return len > valOffset ? QString::fromLocal8Bit(buff + valOffset, len - valOffset) : QString();
}

bool ParserState::readNextLine(bool emptyOK) {
    CHECK_OP_EXT(si, len = 0, false);

    bool ok = false;
    len = io->readLine(buff, READ_BUFF_SIZE, &ok);
    si.setProgress(io->getProgress());

    if (!ok && len == READ_BUFF_SIZE) {
        si.setError(U2::EMBLGenbankAbstractDocument::tr("Line is too long."));
    } else if (len == -1) {
        si.setError(U2::EMBLGenbankAbstractDocument::tr("IO error."));
    }
    return (len > 0 || (emptyOK && ok));
}

QString ParserState::key() const {   
    //assert(len > 0);
    return QString::fromLocal8Bit(buff, qMin(valOffset - 1, len));
}

}//namespace
