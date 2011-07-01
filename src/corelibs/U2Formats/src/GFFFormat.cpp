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

#include "GFFFormat.h"
#include "DocumentFormatUtils.h"

#include <U2Core/IOAdapter.h>
#include <U2Core/Task.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/GAutoDeleteList.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectReference.h>


namespace U2{

#define READ_BUFF_SIZE 4096         //file reader buffer size
#define SEQUENCE_TAG " sequence"    //tag for sequence object
#define FEATURES_TAG " features"    //tag for annotation table
#define LINE_LEN 70                 //line length for 

GFFFormat::GFFFormat(QObject* p):DocumentFormat(p, DocumentFormatFlags_SW, QStringList("gff")){
    formatName = tr("GFF");
    supportedObjectTypes+=GObjectTypes::ANNOTATION_TABLE;
    supportedObjectTypes+=GObjectTypes::SEQUENCE;
}


Document* GFFFormat::loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode){
    if( NULL == io || !io->isOpen() ) {
        ti.setError(L10N::badArgument("IO adapter"));
        return NULL;
    }
    QList<GObject*> objects;

    load(io, objects, fs, ti);

    if (ti.hasError() || ti.cancelFlag) {
        qDeleteAll(objects);
        return NULL;
    }

    Document* doc = new Document( this, io->getFactory(), io->getURL(), objects);
    return doc;
}

void validateHeader( QStringList words){
    bool isOk = false;
    if(words.size() < 2){
        ioLog.info(GFFFormat::tr("Parsing error: invalid header"));
    }
    words[0] = words[0].remove("#");
    if(!words[0].startsWith("gff-version")){
        ioLog.info(GFFFormat::tr("Parsing error: file does not contain version header"));
    }else{
        int ver = words[1].toInt(&isOk);
        if(!isOk){
            ioLog.info(GFFFormat::tr("Parsing error: format version is not an integer"));
        }
        //is version supported
        if(ver != 3){
            ioLog.info(GFFFormat::tr("Parsing error: GFF version %1 is not supported").arg(ver));
        }
    }
}

static QMap<QString, QString> initEscapeCharactersMap() {
    QMap<QString, QString> ret;
    ret[";"] = "%3B";
    ret["="] = "%3D";
    ret[","] = "%2C";
    ret["\t"] = "%09";
    ret["%"] = "%25";
    return ret;
}
static const QMap<QString, QString> escapeCharacters = initEscapeCharactersMap();

static QString escapeBadCharacters(const QString & val) {
    QString ret(val);
    foreach(const QString & key, escapeCharacters.keys()) {
        ret.replace(key, escapeCharacters.value(key));
    }
    return ret;
}

static QString fromEscapedString( const QString & val ) {
    QString ret(val);
    foreach( const QString & val, escapeCharacters.values() ) {
        ret.replace(val, escapeCharacters.key(val));
    }
    return ret;
}

void GFFFormat::load(IOAdapter* io, QList<GObject*>& objects, const QVariantMap& hints, TaskStateInfo& si){
    gauto_array<char> buff = new char[READ_BUFF_SIZE];
    int len = io->readLine(buff.data, READ_BUFF_SIZE);
    buff.data[len] = '\0';
    QString qstrbuf(buff.data);
    QStringList words = qstrbuf.split(QRegExp("\\s+"));
    bool isOk;
    QSet <AnnotationTableObject*> atoSet;
    QMap <QString, DNASequenceObject*> seqMap;
    //header validation
    validateHeader(words);

    int lineNumber = 1;
    QMap<QString,Annotation*> joinedAnnotations;
    bool fastaSectionStarts = false;
    QString headerName, objName;
    QByteArray seq;
    QSet<QString> names;
    while((len = io->readLine(buff.data, READ_BUFF_SIZE)) > 0){
        qstrbuf.clear();
        buff.data[len] = '\0';
        qstrbuf = QString(buff.data);
        //retrieving annotations from  document
        words = parseLine(qstrbuf);
        if(fastaSectionStarts){
            if(words[0].startsWith(">") && headerName.isEmpty()){
                headerName = words.join(" ").remove(">");
            }else if(words[0].startsWith(">")){
                headerName = TextUtils::variate(headerName, "_", names);
                names.insert(headerName);
                objName = headerName + SEQUENCE_TAG;
                DNASequence sequence(objName, seq);
                sequence.info.insert(DNAInfo::FASTA_HDR, objName);
                sequence.info.insert(DNAInfo::ID, objName);
                DNASequenceObject *dnaso = DocumentFormatUtils::addSequenceObject(objects, objName, sequence, hints, si);
                if (si.hasError()) {
                    assert(dnaso = NULL);
                    return;
                }
                seqMap.insert(objName, dnaso);
                headerName = words.join(" ").remove(">");
                seq = "";
            } else {
                if(words.size() > 1){
                    si.setError(tr("Parsing error: sequence in FASTA sequence has whitespaces at line %1").arg(lineNumber));
                    return;
                }
                seq.append(words[0]);
            }
        } else if (!words[0].startsWith("#")){
            if(words.size() != 9){
                si.setError(tr("Parsing error: too few fields at line %1").arg(lineNumber));
                return;
            }
            //annotation's region
            int start = words[3].toInt(&isOk);
            if(!isOk){
                si.setError(tr("Parsing error: start position at line %1 is not integer").arg(lineNumber));
                return;
            }

            int end = words[4].toInt(&isOk);
            if(!isOk){
                si.setError(tr("Parsing error: end position at line %1 is not integer").arg(lineNumber));
                return;
            }

            if(start > end){
                si.setError(tr("Parsing error: incorrect annotation region at line %1").arg(lineNumber));
                return;
            }

            start--;
            U2Region range(start, end - start);

            QString groupName = words[2];
            QString annName = groupName; //by default annotation named as group
            //annotation's qualifiers from attributes
            SharedAnnotationData d(new AnnotationData());
            Annotation *a = NULL;
            bool newJoined = false;
            QString id;
            if(words[8] != "."){
                QStringList pairs = words[8].split(";", QString::SkipEmptyParts);
                foreach(QString p, pairs){
                    QStringList qual = p.split("=", QString::SkipEmptyParts);
                    if(qual.size() != 2){
                        si.setError(tr("Parsing error: one of the qualifiers in attributes are incorrect at line %1").arg(lineNumber));
                        return;
                    }
                    qual[0] = fromEscapedString(qual[0]);
                    qual[1] = fromEscapedString(qual[1]);
                    if(qual[0] == "name"){
                        annName = qual[1];
                    }else{
                        d->qualifiers.append(U2Qualifier(qual[0], qual[1]));
                        if(qual[0] == "ID"){
                            id = qual[1];
                            if(joinedAnnotations.contains(id)){
                                a = *(joinedAnnotations.find(id));
                                bool hasIntersections = range.findIntersectedRegion(a->getRegions())!=-1;
                                if(hasIntersections){
                                    ioLog.info(tr("Wrong location for joined annotation at line %1. Line was skipped.").arg(lineNumber));
                                } else {
                                    a->addLocationRegion(range);
                                }
                            }else{
                                newJoined = true;
                            }
                        }
                    }
                }
            }
            
            //if annotation joined, don't rewrite it data
            if(a == NULL){
                a = new Annotation(d);
                if(newJoined){
                    joinedAnnotations.insert(id, a);
                }
                a->addLocationRegion(range);
                a->setAnnotationName(annName);		

                QString atoName = words[0] + FEATURES_TAG;
                AnnotationTableObject *ato = NULL;
                foreach(GObject *ob, objects){
                    if(ob->getGObjectName() == atoName){
                        ato = (AnnotationTableObject *)ob;
                    }
                }
                if(!ato){
                    ato = new AnnotationTableObject(atoName);
                    objects.append(ato);
                    atoSet.insert(ato);
                }

                //qualifiers from columns
                if(words[1] != "."){
                    a->addQualifier("source", words[1]);
                }

                if(words[5] != "."){
                    a->addQualifier("score", words[5]);
                }

                if(words[7] != "."){
                    a->addQualifier("phase", words[7]);
                }

                //strand detection
                if(words[6] == "-"){
                    a->setStrand(U2Strand::Complementary);
                }

                ato->addAnnotation(a, groupName);
            }
        }else{
            if(words[0].startsWith("##fasta", Qt::CaseInsensitive)){
                fastaSectionStarts = true;
            }
        }
        lineNumber++;
    }

    //handling last fasta sequence
    if(fastaSectionStarts){
        headerName = TextUtils::variate(headerName, "_", names);
        names.insert(headerName);
        objName = headerName + SEQUENCE_TAG;
        DNASequence sequence(objName, seq);
        sequence.info.insert(DNAInfo::FASTA_HDR, objName);
        sequence.info.insert(DNAInfo::ID, objName);
        DNASequenceObject *dnaso = DocumentFormatUtils::addSequenceObject(objects, objName, sequence, hints, si);
        if (si.hasError()) {
            assert(dnaso == NULL);
            qDeleteAll(seqMap.values());
            seqMap.clear();
            return;
        }
        seqMap.insert(objName, dnaso);
    }
    
    //linking annotation tables with corresponding sequences
    foreach(AnnotationTableObject *ob, atoSet){
        QString objName = ob->getGObjectName();
        objName.replace(FEATURES_TAG, SEQUENCE_TAG);
        if(seqMap.contains(objName)){
            GObjectReference sequenceRef(GObjectReference(io->getURL().getURLString(), "", GObjectTypes::SEQUENCE));
            sequenceRef.objName = objName;
            ob->addObjectRelation(GObjectRelation(sequenceRef, GObjectRelationRole::SEQUENCE));
        }
    }
}

RawDataCheckResult GFFFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    const char* data = rawData.constData();
    int size = rawData.size();
    int n = TextUtils::skip(TextUtils::WHITES, data, size);
    int newSize = size - n;

    bool hasBinaryData = TextUtils::contains(TextUtils::BINARY, data, size);
    if(hasBinaryData){
        return FormatDetection_NotMatched;
    }

    if (newSize <= 13) {
        return FormatDetection_NotMatched;
    }
    QString header(rawData);
    header = header.remove("#");
    FormatDetectionScore res = FormatDetection_NotMatched;
    if(header.startsWith("gff-version")){
        res = FormatDetection_Matched;
    }

    QString dataStr(rawData);
    QStringList qsl = dataStr.split("\n");
    foreach(QString str, qsl){
        if(!str.startsWith("#")){
            QStringList l = parseLine(str);
            if(l.size() == 9){
                bool b3, b4;
                l[3].toInt(&b3);
                l[4].toInt(&b4);
                if (b3 && b4){
                    res = qMax(res, FormatDetection_HighSimilarity) ;
                } else {
                    return FormatDetection_NotMatched;
                }                
            }
        }
    }

    return res;
}

QStringList GFFFormat::parseLine( QString line ) const{
    QChar prev('a'); //as default value not empty char
    QString pair;
    QStringList result;
    QString word;

    foreach(QChar c, line){
        pair.clear();
        pair.append(prev);
        pair.append(c);
        if((c == '\t') || (pair == " \t") || (pair == "  ")){
            if((word != "  ") && (word != " ") && !word.isEmpty()){
                result.append(word);
            }
            word.clear();
        }else{
            word.append(c);
        }
        prev = c;
    }
    if((word != "  ") && (word != " ") && !word.isEmpty()){
        result.append(word);
    }
    return result;
}

void GFFFormat::storeDocument( Document* doc, TaskStateInfo& si, IOAdapter* io ){
    QByteArray header("##gff-version\t3\n");
    qint64 len = io->writeBlock(header);
    if (len!=header.size()) {
        si.setError(L10N::errorWritingFile(doc->getURL()));
        return;
    }
    QByteArray qbaRow;
    QList<GObject*> atos = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);
    int joinID = 0;
    QSet<QString> knownIDs;
    QStringList cleanRow;
    for(int i = 0; i != 9; i++){
        cleanRow.append(".");
    }
    foreach(GObject *ato , atos){
         QList<Annotation*> aList = (qobject_cast<AnnotationTableObject*>(ato))->getAnnotations();
         //retrieving known IDs
         foreach(const Annotation *ann, aList){
             if(ann->isValidQualifierName("ID")){
                knownIDs.insert(ann->findFirstQualifierValue("ID"));
             }
         }
         foreach(const Annotation *ann, aList){
            QStringList row = cleanRow;
            QVector<U2Region> location = ann->getRegions();
            QVector<U2Qualifier> qualVec = ann->getQualifiers();
            //generating unique ID for joined annotation
            if((location.size() > 1) && !ann->isValidQualifierName("ID")){
                for(;knownIDs.contains(QString::number(joinID));joinID++);
                qualVec.append(U2Qualifier("ID", QString::number(joinID)));
            }
            foreach(const U2Region r, location){
                QString name = ato->getGObjectName();
                row[0] = name.left(name.size() - QString(FEATURES_TAG).size()); //removing previously added tag
                //filling strand field
                if(ann->getStrand().isCompementary()){
                    row[6] = "-";
                }
                //filling location fields
                row[3] = QString::number(r.startPos + 1);
                row[4] = QString::number(r.endPos());
                row[2] = (ann->getGroups().first())->getGroupName();
                QString additionalQuals = "name=" + escapeBadCharacters(ann->getAnnotationName());
                //filling fields with qualifiers data
                foreach(U2Qualifier q, qualVec){
                    if(q.name == "source"){
                        row[1] = q.value;
                    }else if(q.name == "score"){
                        row[5] = q.value;
                    }else if(q.name == "phase"){
                        row[7] = q.value;
                    }else{
                        additionalQuals.append(";" + escapeBadCharacters(q.name) + "=" + escapeBadCharacters(q.value));
                    }
                }
                row[8] = additionalQuals;
                qbaRow = row.join("\t").toAscii() + "\n";
                qint64 len = io->writeBlock(qbaRow);
                if (len!=qbaRow.size()) {
                    si.setError(L10N::errorWritingFile(doc->getURL()));
                    return;
                }
            }
         }
    }
    QList<GObject*> sequences = doc->findGObjectByType(GObjectTypes::SEQUENCE);
    if(!sequences.isEmpty()){
        qbaRow = "##FASTA\n";
        if (io->writeBlock(qbaRow) != qbaRow.size()) {
            si.setError(L10N::errorWritingFile(doc->getURL()));
            return;
        }
        foreach(GObject *s, sequences){
            DNASequenceObject *dnaso = qobject_cast<DNASequenceObject*>(s);
            QString fastaHeader = dnaso->getGObjectName();
            int tagSize = QString(SEQUENCE_TAG).size(), headerSize = fastaHeader.size();
            fastaHeader = fastaHeader.left(headerSize - tagSize);  //removing previously added tag
            fastaHeader.prepend(">");
            fastaHeader.append( '\n' );
            qbaRow = fastaHeader.toAscii();
            if (io->writeBlock(qbaRow) != qbaRow.size()) {
                si.setError(L10N::errorWritingFile(doc->getURL()));
                return;
            }
            
            const char* seq = dnaso->getSequence().constData();
            int len = dnaso->getSequence().length();
            for (int i = 0; i < len; i += LINE_LEN ) {
                int chunkSize = qMin( LINE_LEN, len - i );
                if (io->writeBlock( seq + i, chunkSize ) != chunkSize || !io->writeBlock( "\n", 1 )) {
                    si.setError(L10N::errorWritingFile(doc->getURL()));
                    return;
                }
            }
        }
    }
}

} //namespace

