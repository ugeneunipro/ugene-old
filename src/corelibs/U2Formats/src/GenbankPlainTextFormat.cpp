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

#include "GenbankPlainTextFormat.h"
#include "GenbankLocationParser.h"
#include "GenbankFeatures.h"
#include "DocumentFormatUtils.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/Task.h>
#include <U2Core/L10n.h>

#include <U2Core/DNAInfo.h>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectRelationRoles.h>

#include <U2Core/TextUtils.h>
#include <U2Core/QVariantUtils.h>

#include <memory>

namespace U2 {

/* TRANSLATOR U2::GenbankPlainTextFormat */
/* TRANSLATOR U2::EMBLGenbankAbstractDocument */
/* TRANSLATOR U2::IOAdapter */

GenbankPlainTextFormat::GenbankPlainTextFormat(QObject* p)
: EMBLGenbankAbstractDocument(BaseDocumentFormats::PLAIN_GENBANK, tr("Genbank"), 79, DocumentFormatFlags_SW, p)
{
    fileExtensions << "gb" << "gbk" << "gen" << "genbank";
    sequenceStartPrefix = "ORIGIN";
    fPrefix = "  ";
}

FormatDetectionResult GenbankPlainTextFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    //TODO: improve handling
    const char* data = rawData.constData();
    int size = rawData.size();
    bool textOnly = !TextUtils::contains(TextUtils::BINARY, data, size);
    if (!textOnly || size < 100) {
        return FormatDetection_NotMatched;
    }
    bool startsWithLocus = TextUtils::equals("LOCUS ", data, 6);
    return startsWithLocus ? FormatDetection_VeryHighSimilarity : FormatDetection_NotMatched;
}


bool GenbankPlainTextFormat::readIdLine(ParserState* st) {
    if (!st->hasKey("LOCUS")) {
        st->si.setError(tr("LOCUS is not the first line"));
        return false;
    }

    QString locusStr = st->value();
    QStringList tokens = locusStr.split(" ", QString::SkipEmptyParts);
    if (tokens.isEmpty()) {
        st->si.setError(tr("Error parsing LOCUS line"));
        return false;
    }
    // try improving name readability
    tokens[0] = tokens[0].replace(QRegExp("_(?![0-9])"), QChar(' '));
    st->entry->name = tokens[0];

    if (tokens.size() >= 3 && tokens[2] == "bp") {
        QString len = tokens[1];
        st->entry->seqLen = len.toInt();
    }
    if (tokens.size() == 7) {
        // seems to be canonical header
        DNALocusInfo loi;
        loi.name = tokens[0];
        loi.topology = tokens[4];
        loi.molecule = tokens[3];
        loi.division = tokens[5];
        loi.date = tokens[6];
        st->entry->tags.insert(DNAInfo::LOCUS, qVariantFromValue<DNALocusInfo>(loi));
        st->entry->circular = loi.topology == "circular";
    } else {
        st->entry->tags.insert(DNAInfo::ID, tokens[0]);
        st->entry->tags.insert(DNAInfo::EMBL_ID, locusStr);
        st->entry->circular = locusStr.contains("circular");
    }
    return true;
}



bool GenbankPlainTextFormat::readEntry(QByteArray& sequence, ParserState* st) {
    TaskStateInfo& si = st->si;
    QString lastTagName;
    bool hasLine = false;
    while (hasLine || st->readNextLine(true)) {
        hasLine = false;
        if (st->len == 0) {
            continue;
        }
        if (st->isNull()) {
            readIdLine(st);
            assert(si.hasErrors() || !st->entry->name.isEmpty());
            continue;
        }
        if (st->hasKey("FEATURES") && st->readNextLine()) {
            readAnnotations(st, sequence.size());
            hasLine = true;
            continue;
        }
        if (st->hasKey("SOURCE")) {
            DNASourceInfo soi;
            soi.name = st->value();
            while (st->readNextLine() && st->hasContinuation()) {
                soi.name.append(st->value());
            }
            if (st->hasKey("  ORGANISM")) {
                soi.organism = st->value();
                while (st->readNextLine() && st->hasContinuation()) {
                    soi.taxonomy.append(st->value());
                }
            } else {
                st->si.setError(tr("incomplete SOURCE record"));
                break;
            }
            st->entry->tags.insertMulti(DNAInfo::SOURCE, qVariantFromValue<DNASourceInfo>(soi));
            hasLine = true;
            continue;
        }
        if (st->hasKey("REFERENCE")) {
            while (st->readNextLine() && (st->hasValue() && st->buff[0] == ' '))
            {
                //TODO
            }
            hasLine = true;
            continue;
        }
        if (st->hasKey("ACCESSION") || (st->hasContinuation() && lastTagName == "ACCESSION")) {
            QVariant v = st->entry->tags.value(DNAInfo::ACCESSION);
            st->entry->tags[DNAInfo::ACCESSION] = QVariantUtils::addStr2List(v, st->value().split(" "));
            continue;
        }
        if (TextUtils::equals(st->buff, "//", 2)) {
            // end of entry
            return true;
        }
        if (st->hasKey("ORIGIN")) {
            if (st->hasValue()) {
                st->entry->tags.insert(DNAInfo::ORIGIN, st->value());
            }
            if (st->readNextLine() && st->hasKey("CONTIG")) {
                QStringList s(st->value());
                while (st->readNextLine() && st->hasContinuation()) {
                    s.append(st->value());
                }
                st->entry->tags.insert(DNAInfo::CONTIG, s);
                if (!TextUtils::equals(st->buff, "//", 2)) {
                    break;
                }
            } else {
                if (st->len >0) {
                    st->io->skip(-st->len - 1);
                }
                readSequence(sequence, st);
            }
            return true;
        }

        if (st->hasContinuation()) {
            QVariant v = st->entry->tags.take(lastTagName);
            v = QVariantUtils::addStr2List(v, st->value());
            st->entry->tags.insert(lastTagName, v);
        } else if (st->hasValue()) {
            lastTagName = st->key().trimmed();
            st->entry->tags.insertMulti(lastTagName, st->value());
        }
    }
    if (!st->isNull() && !si.hasErrors() && !si.cancelFlag) {
        si.setError(U2::EMBLGenbankAbstractDocument::tr("Record is truncated."));
    }

    return false;
}

//////////////////////////////////////////////////////////////////////////
/// saving

static QString genLocusString(QList<GObject*> aos, DNASequenceObject* so);
static void writeAnnotations(IOAdapter* io, QList<GObject*> aos, TaskStateInfo& si);
static void writeSequence(IOAdapter* io, DNASequenceObject* ao, TaskStateInfo& si);
static void prepareMultiline(QString& lineToChange, int spacesOnLineStart, bool newLineAtTheEnd = true, int maxLineLen = 79);

#define VAL_OFF 12
static bool writeKeyword(IOAdapter* io, TaskStateInfo& si, const QString& key, const QString& value, bool wrap = true /*TODO*/) {
    Q_UNUSED(wrap);
    try {
        assert(key.length() < VAL_OFF);
        int klen = qMin(VAL_OFF - 1, key.length());
        qint64 len = io->writeBlock(key.left(klen).toLocal8Bit());
        if (len!=klen) {
            throw 0;
        }
        static char spaces[] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
        int slen = VAL_OFF - klen;
        len = io->writeBlock(spaces, slen);
        if (len!=slen) {
            throw 0;
        }

        len = io->writeBlock(value.toLocal8Bit());
        if (len!=value.size()) {
            throw 0;
        }

        static char eol[] = {'\n'};
        if (!io->writeBlock(eol, 1)) {
            throw 0;
        }

    } catch( int ) {
        si.setError(GenbankPlainTextFormat::tr("Error writing document"));
        return false;
    }
    return true;
}

typedef QPair<QString, QString> StrPair;
static QList<StrPair> formatKeywords(DNASequenceObject* so) {
    QList<StrPair> res;
    QMultiMap<QString, QVariant> tags(so->getDNASequence().info);

    tags.remove(DNAInfo::LOCUS);
    tags.remove(DNAInfo::ID);
    tags.remove(DNAInfo::CONTIG);
    tags.remove(DNAInfo::ORIGIN);
    tags.remove(EMBLGenbankAbstractDocument::UGENE_MARK);
    if (tags.contains(DNAInfo::ACCESSION)) {
        tags.insert(DNAInfo::ACCESSION, tags.take(DNAInfo::ACCESSION).toStringList().join(" "));
    }

    QStringList order;
    order << DNAInfo::DEFINITION << DNAInfo::ACCESSION << DNAInfo::VERSION;
    order << DNAInfo::PROJECT << DNAInfo::KEYWORDS << DNAInfo::SEGMENT;
    foreach(const QString& key, order) {
        while (tags.contains(key)) {
            QVariant v = tags.take(key);
            if (v.canConvert(QVariant::String)) {
                res << qMakePair(key, v.toString());
            } else if (v.canConvert(QVariant::StringList)) {
                QStringList l = v.toStringList();
                if (l.size() == 0) {
                    assert(0);
                    continue;
                }
                res << qMakePair(key, l.takeFirst());
                foreach(const QString& s, l) {
                    res << qMakePair(QString(), s);
                }
            } else {
                assert(0);
            }
        }
    }
    {
        QString key = DNAInfo::SOURCE;
        while (tags.contains(key)) {
            QVariant v = tags.take(key);
            DNASourceInfo soi = v.value<DNASourceInfo>();
            res << qMakePair(key, soi.name);
            if (!soi.organelle.isEmpty()) {
                res[res.size() - 1].second += " " + soi.organelle;
            }
            res << qMakePair(QString("  ORGANISM"), soi.organism);
            foreach(const QString& s, soi.taxonomy) {
                res << qMakePair(QString(), s);
            }
        }
    }
    {
        QString key = DNAInfo::REFERENCE;
        while (tags.contains(key)) {
            QVariant v = tags.take(key);
            DNAReferenceInfo ri = v.value<DNAReferenceInfo>();
            //res << qMakePair(key, v.toString());
        }
    }

    QMapIterator<QString, QVariant> it(tags);
    while (it.hasNext())
    {
        it.next();
        if (it.value().type() == QVariant::String) {
            res << qMakePair(it.key(), it.value().toString());
        } else if (it.value().type() == QVariant::StringList) {
            QStringList l = it.value().toStringList();
            if (l.size() == 0) {
                assert(0);
                continue;
            }
            res << qMakePair(it.key(), l.takeFirst());
            foreach(const QString& s, l) {
                res << qMakePair(QString(), s);
            }
        } else {
            assert(0);
        }
    }
    return res;
}

void GenbankPlainTextFormat::storeDocument( Document* doc, TaskStateInfo& si, IOAdapter* io ) {
    QList<GObject*> seqs = doc->findGObjectByType(GObjectTypes::SEQUENCE);
    QList<GObject*> anns = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);

    while (!seqs.isEmpty() || !anns.isEmpty()) {

        DNASequenceObject* so = seqs.isEmpty() ? NULL : static_cast<DNASequenceObject*>(seqs.takeFirst());
        QList<GObject*> aos;
        if (so) {
            if (!anns.isEmpty()) {
                aos = GObjectUtils::findObjectsRelatedToObjectByRole(so, GObjectTypes::ANNOTATION_TABLE, GObjectRelationRole::SEQUENCE, anns, UOF_LoadedOnly);
                foreach(GObject* o, aos) {
                    anns.removeAll(o);
                }
            }
        } else {
            assert(!anns.isEmpty());
            aos << anns.takeFirst();
        }

        // write mandatory locus string
        QString locusString = genLocusString(aos, so);
        if (!writeKeyword(io, si, DNAInfo::LOCUS, locusString, false)) {
            return;
        }
        // write other keywords
        if (so) {
            QList<StrPair> lst(formatKeywords(so));
            foreach (const StrPair& p, lst) {
                if (!writeKeyword(io, si, p.first, p.second)) {
                    return;
                }
            }
        }

        //write tool mark
        QList<GObject*> annsAndSeqObjs; annsAndSeqObjs<<aos; if (so!=NULL) {annsAndSeqObjs<<so;}
        if (!annsAndSeqObjs.isEmpty()) {
            //write object names, TODO: avoid writing if names are default?
            if (!writeKeyword(io, si, UGENE_MARK, annsAndSeqObjs[0]->getGObjectName(), false)) {
                return;
            }
            for (int x=1; x < annsAndSeqObjs.size(); x++) {
                if (!writeKeyword(io, si, QString(), annsAndSeqObjs[x]->getGObjectName(), false)) {
                    return;
                }
            }
        }

        // write annotations
        if (!aos.isEmpty()) {
            writeAnnotations(io, aos, si);
            if (si.hasErrors()) {
                return;
            }
        }

        if (so) {
            //todo: store sequence alphabet!
            writeSequence(io, so, si);
            if (si.hasErrors()) {
                return;
            }
        }

        // write last line marker
        QByteArray lastLine("//\n");
        qint64 len = io->writeBlock(lastLine);
        if (len!=lastLine.size()) {
            si.setError(L10N::errorWritingFile(doc->getURL()));
            return;
        }
    }
}

static QString getDate(){
    const char* MonthsInEng[] = {" ", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    return QDate::currentDate().toString("dd-") +
        MonthsInEng[QDate::currentDate().month()] +
        QDate::currentDate().toString("-yyyy");
}

static QString padToLen(const QString& s, int width) {
    if (width > s.length()) {
        return s.leftJustified(width);
    } else {
        return s + " ";
    }
}

static QString genLocusString(QList<GObject*> aos, DNASequenceObject* so) {
    QString loc, date;
    if (so) {
        const DNASequence& dna = so->getDNASequence();
        QString len = QString::number(dna.length());
        loc = dna.getName();
        if (loc.isEmpty()) {
            loc = so->getGObjectName();
        }
        loc = padToLen(loc.replace(QChar(' '), QChar('_')), qMax(0, 28 - len.length()));
        loc.append(len).append(" bp ");
        if (dna.info.contains(DNAInfo::LOCUS)) {
            DNALocusInfo loi = dna.info.value(DNAInfo::LOCUS).value<DNALocusInfo>();
            assert(!loi.name.isEmpty());
            QString& mol = loi.molecule;
            if (mol.size() >= 3 && mol.at(2) != '-') loc.append("   ");
            loc = padToLen(loc.append(mol), 43);
            loc = padToLen(loc.append(loi.topology), 52);
            loc = loc.append(loi.division);
            date = loi.date;
        } else {
            if (so->isCircular()) {
                loc = padToLen(loc.append("CIRCULAR"), 52);
            }
        }
    } else {
        assert(!aos.isEmpty());
        loc = !aos.isEmpty() ? aos.takeFirst()->getGObjectName() : "unknown"; //FIXME ???
    }
    assert(!loc.isEmpty());
    if (date.isEmpty()) {
        date = getDate();
    }
    loc = padToLen(loc, 56) + date;
    return loc;
}

static void writeQualifier(const QString& name, const QString& val, IOAdapter* io, TaskStateInfo& si, const char* spaceLine) {
    int len = io->writeBlock(spaceLine, 21);
    if (len != 21) {
        si.setError(GenbankPlainTextFormat::tr("Error writing document"));
        return;
    }
    QString qstr;
    bool num; val.toInt(&num);
    if (num) {
        qstr = "/"+name+ "="+val;
    } else {
        qstr = "/"+name+ "=\""+val+"\"";
    }
    prepareMultiline(qstr, 21);
    len = io->writeBlock(qstr.toLocal8Bit());
    if (len != qstr.length()) {
        si.setError(GenbankPlainTextFormat::tr("Error writing document"));
    }
}

static void writeAnnotations(IOAdapter* io, QList<GObject*> aos, TaskStateInfo& si) {
    assert(!aos.isEmpty());
    QByteArray header("FEATURES             Location/Qualifiers\n");

    //write "FEATURES"
    qint64 len = io->writeBlock(header);
    if (len!=header.size()) {
        si.setError(GenbankPlainTextFormat::tr("Error writing document"));
        return;
    }

    //write every feature
    const char* spaceLine = TextUtils::SPACE_LINE.data();
    const QByteArray& aminoQ = GBFeatureUtils::QUALIFIER_AMINO_STRAND;
    const QByteArray& aminoQYes = GBFeatureUtils::QUALIFIER_AMINO_STRAND_YES;
    const QByteArray& aminoQNo = GBFeatureUtils::QUALIFIER_AMINO_STRAND_NO;
    const QByteArray& nameQ = GBFeatureUtils::QUALIFIER_NAME;
    const QByteArray& groupQ = GBFeatureUtils::QUALIFIER_GROUP;
    const QString& defaultKey = GBFeatureUtils::DEFAULT_KEY;

    QList<Annotation*> sortedAnnotations;
    foreach(GObject* o, aos) {
        AnnotationTableObject* ao = qobject_cast<AnnotationTableObject*>(o);
        assert(ao);
        sortedAnnotations += ao->getAnnotations();
    }

    qStableSort(sortedAnnotations.begin(), sortedAnnotations.end(), annotationLessThanByRegion);
    for(int i = 0; i < sortedAnnotations.size(); ++i) {
        Annotation * a = sortedAnnotations.at(i); assert(a != NULL);
        
        //write name of the feature
        len = io->writeBlock(spaceLine, 5);
        if (len != 5) {
            si.setError(GenbankPlainTextFormat::tr("Error writing document"));
            return;
        }
        QString aName = a->getAnnotationName();
        
        GBFeatureKey key = GBFeatureUtils::getKey(aName);
        const QString& keyStr = key == GBFeatureKey_UNKNOWN ? defaultKey: aName;
        len = io->writeBlock(keyStr.toLocal8Bit());
        if (len!=keyStr.length()) {
            si.setError(GenbankPlainTextFormat::tr("Error writing document"));
            return;
        }
        int nspaces = 22 - keyStr.length() - 6;
        assert(nspaces > 0);
        len = io->writeBlock(spaceLine, nspaces);
        if (len !=nspaces) {
            si.setError(GenbankPlainTextFormat::tr("Error writing document"));
            return;
        }

        //write location
        QString multiLineLocation = Genbank::LocationParser::buildLocationString(a->data());
        prepareMultiline(multiLineLocation, 21);
        len = io->writeBlock(multiLineLocation.toLocal8Bit());
        if (len != multiLineLocation.size()) {
            si.setError(GenbankPlainTextFormat::tr("Error writing document"));
            return;
        }

        //write qualifiers
        foreach (const U2Qualifier& q, a->getQualifiers()) {
            writeQualifier(q.name, q.value, io, si, spaceLine);
            if (si.hasErrors()) {
                return;
            }
        }

        //write name if its not the same as a name
        if (key == GBFeatureKey_UNKNOWN) {
            writeQualifier(nameQ, aName, io, si, spaceLine);
        }

        //write strand info
        //if (a->getAminoFrame() != TriState_Unknown) {
        //    const QString& val = a->getAminoFrame() == TriState_No ? aminoQNo : aminoQYes;
        //    writeQualifier(aminoQ, val, io, si, spaceLine);
        //}

        //write group
        const QList<AnnotationGroup*>& groups = a->getGroups();
        bool storeGroups = true;
        if (groups.size() == 1) {
            AnnotationGroup* ag = groups.first();
            storeGroups = !ag->isTopLevelGroup() || ag->getGroupName()!=aName;
        }
        if (storeGroups) {
            foreach(AnnotationGroup* ag, a->getGroups()) {
                writeQualifier(groupQ, ag->getGroupPath(), io, si, spaceLine);
            }
        }
    }
}

static void writeSequence(IOAdapter* io, DNASequenceObject* ao, TaskStateInfo& si) {
    static const int charsInLine = 60;

    const QByteArray& seq = ao->getSequence();
    int slen = seq.length();
    const char* sequence = seq.constData();
    const char* spaces = TextUtils::SPACE_LINE.constData();
    QByteArray num;
    bool ok = true;
    int blen = io->writeBlock(QByteArray("ORIGIN\n"));
    if (blen != 7) {
        si.setError(L10N::errorWritingFile(ao->getDocument()->getURL()));
        return;
    }
    for (int pos = 0; pos < slen; pos+=charsInLine) {
        num.setNum(pos+1);

        //right spaces
        blen = 10 - num.length()-1;
        int l = (int)io->writeBlock(QByteArray::fromRawData(spaces, blen));
        if (l!=blen) {
            ok = false;
            break;
        }

        //current pos
        l = (int)io->writeBlock(num);
        if (l != num.length()) {
            ok = false;
            break;
        }

        //sequence
        int last = qMin(pos+charsInLine, slen);
        for (int j=pos; j < last; j+=10) {
            l = (int)io->writeBlock(QByteArray::fromRawData(" ", 1));
            if (l != 1) {
                ok = false;
                break;
            }
            int chunkLen = qMin(10, slen - j);
            l = io->writeBlock(QByteArray::fromRawData(sequence + j, chunkLen));
            if (l!=chunkLen) {
                ok = false;
                break;
            }
        }
        if (!ok) {
            break;
        }

        //line end
        l = (int)io->writeBlock(QByteArray("\n", 1));
        if (l != 1) {
            ok = false;
            break;
        }
    }
    if (!ok) {
        si.setError(L10N::errorWritingFile(ao->getDocument()->getURL()));
    }
}


// splits line into multiple lines adding 'spacesOnLineStart' to every line except first one
// and '\n' to the end of every new line
static void prepareMultiline(QString& line, int spacesOnLineStart, bool newLineAtTheEnd, int maxLineLen) {
    Q_ASSERT(spacesOnLineStart < maxLineLen);
    const int len = line.length() ;
    if (spacesOnLineStart + len > maxLineLen) {
        QByteArray spacesPrefix(spacesOnLineStart, ' ');
        QString newLine;
        int charsInLine = maxLineLen - spacesOnLineStart;
        int pos = 0;
        do {
            if (pos!=0) {
                newLine.append('\n');
                newLine.append(spacesPrefix);
            }
            int pos2 =  pos + charsInLine - 1;
            if (pos2 < len) { //not the last line
                while (pos2 > pos && !line[pos2].isSpace() && (line[pos2]!= ',')) {
                    pos2--;
                }
                if (pos == pos2) { //we failed to find word end
                    pos2 = pos + charsInLine - 1;
                }
                newLine.append(line.mid(pos, pos2 + 1 - pos));
            } else { //last line
                newLine.append(line.mid(pos, len-pos));
            }
            pos=pos2+1;
        } while (pos<len);
        line = newLine;
    }
    if (newLineAtTheEnd) {
        line+="\n";
    }
}

}//namespace
