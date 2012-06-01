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

#include "MSFFormat.h"

#include <U2Formats/DocumentFormatUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2AlphabetUtils.h>
#include <U2Core/U2DbiUtils.h>

namespace U2 {

const int MSFFormat::CHECK_SUM_MOD = 10000;
const QByteArray MSFFormat::MSF_FIELD = "MSF:";
const QByteArray MSFFormat::CHECK_FIELD = "Check:";
const QByteArray MSFFormat::LEN_FIELD = "Len:";
const QByteArray MSFFormat::NAME_FIELD = "Name:";
const QByteArray MSFFormat::TYPE_FIELD = "Type:";
const QByteArray MSFFormat::WEIGHT_FIELD = "Weight:";
const QByteArray MSFFormat::TYPE_VALUE_PROTEIN = "P";
const QByteArray MSFFormat::TYPE_VALUE_NUCLEIC = "N";
const double MSFFormat::WEIGHT_VALUE = 1.0;
const QByteArray MSFFormat::END_OF_HEADER_LINE = "..";
const QByteArray MSFFormat::SECTION_SEPARATOR = "//";
const int MSFFormat::CHARS_IN_ROW = 50;
const int MSFFormat::CHARS_IN_WORD = 10;

/* TRANSLATOR U2::MSFFormat */    

//TODO: recheck if it does support streaming! Fix isObjectOpSupported if not!

MSFFormat::MSFFormat(QObject* p) : DocumentFormat(p, DocumentFormatFlag_SupportWriting, QStringList("msf")) {
    formatName = tr("MSF");
    supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
    formatDescription = tr("MSF format is used to store multiple aligned sequences. Files include the sequence name and the sequence itself, which is usually aligned with other sequences in the file.");
}

static bool getNextLine(IOAdapter* io, QByteArray& line) {
    static int READ_BUFF_SIZE = 1024;
    QByteArray readBuffer(READ_BUFF_SIZE, '\0');
    char* buff = readBuffer.data();

    qint64 len;
    bool eolFound = false, eof = false;
    while (!eolFound) {
        len = io->readLine(buff, READ_BUFF_SIZE, &eolFound);
        if (len < READ_BUFF_SIZE && !eolFound) {
            eolFound = eof = true;
        }
        line += readBuffer;
    }
    if (len != READ_BUFF_SIZE) {
        line.resize(line.size() + len - READ_BUFF_SIZE);
    }
    line = line.simplified();
    return eof;
}

static QByteArray getField(const QByteArray& line, const QByteArray& name) {
    int p = line.indexOf(name);
    if (p >= 0) {
        p += name.length();
        if (line[p] == ' ')
            ++p;
        int q = line.indexOf(' ', p);
        if (q >= 0)
            return line.mid(p, q - p);
        else
            return line.mid(p);
    }
    return QByteArray();
}

int MSFFormat::getCheckSum(const QByteArray& seq) {
    int sum = 0;
    static int CHECK_SUM_COUNTER_MOD = 57;
    for (int i = 0; i < seq.length(); ++i) {
        char ch = seq[i];
        if (ch >= 'a' && ch <= 'z') {
            ch = ch + 'A' - 'a';
        }
        sum = (sum + ((i % CHECK_SUM_COUNTER_MOD) + 1) * ch) % MSFFormat::CHECK_SUM_MOD;
    }
    return sum;
}

void MSFFormat::load(IOAdapter* io, QList<GObject*>& objects, U2OpStatus& ti) {
    MAlignment al(io->getURL().baseFileName());

    //skip comments
    int checkSum = -1;
    while (!ti.isCoR() && checkSum < 0) {
        QByteArray line;
        if (getNextLine(io, line)) {
            ti.setError(MSFFormat::tr("Incorrect format"));
            return;
        }
        if (line.endsWith(END_OF_HEADER_LINE)) {
            bool ok;
            checkSum = getField(line, CHECK_FIELD).toInt(&ok);
            if (!ok || checkSum < 0)
                checkSum = CHECK_SUM_MOD;
        }
        ti.setProgress(io->getProgress());
    }

    //read info
    int sum = 0;
    QMap <QString, int> seqs;
    while (!ti.isCoR()) {
        QByteArray line;
        if (getNextLine(io, line)) {
            ti.setError(MSFFormat::tr("Unexpected end of file"));
            return;
        }
        if (line.startsWith(SECTION_SEPARATOR))
            break;

        bool ok = false;
        QString name = QString::fromLocal8Bit(getField(line, NAME_FIELD).data());
        if (name.isEmpty()) {
            continue;
        }
        int check = getField(line, CHECK_FIELD).toInt(&ok);
        if (!ok || check < 0) {
            sum = check = CHECK_SUM_MOD;
        }

        seqs.insert(name, check);
        al.addRow(MAlignmentRow(name));
        if (sum < CHECK_SUM_MOD) {
            sum = (sum + check) % CHECK_SUM_MOD;
        }

        ti.setProgress(io->getProgress());
    }
    if (checkSum < CHECK_SUM_MOD && sum < CHECK_SUM_MOD && sum != checkSum) {
        ti.setError(MSFFormat::tr("Check sum test failed"));
        return;
    }

    //read data
    bool eof = false;
    while (!eof && !ti.isCoR()) {
        QByteArray line;
        eof = getNextLine(io, line);
        if (line.isEmpty()) {
            continue;
        }

        int i = 0, n = al.getNumRows();
        for (; i < n; i++) {
            const MAlignmentRow& row = al.getRow(i);
            QByteArray t = row.getName().toLocal8Bit();
            if (line.startsWith(t) && line[t.length()] == ' ') {
                break;
            }
        }
        if (i == n) {
            continue;
        }
        for (int q, p = line.indexOf(' ') + 1; p > 0; p = q + 1) {
            q = line.indexOf(' ', p);
            QByteArray subSeq = (q < 0) ? line.mid(p) : line.mid(p, q - p);
            al.appendChars(i, subSeq.constData(), subSeq.length());
        }

        ti.setProgress(io->getProgress());
    }

    //checksum
    for (int i=0; i<al.getNumRows(); i++) {
        const MAlignmentRow& row = al.getRow(i);
        int expectedCheckSum = seqs[row.getName()];
        int sequenceCheckSum = getCheckSum(row.toByteArray(al.getLength()));
        if ( expectedCheckSum < CHECK_SUM_MOD &&  sequenceCheckSum != expectedCheckSum) {
            ti.setError(MSFFormat::tr("Check sum test failed"));
            return;
        }
        al.replaceChars(i, '.', MAlignment_GapChar);
        al.replaceChars(i, '~', MAlignment_GapChar);
    }

    U2AlphabetUtils::assignAlphabet(al);
    CHECK_EXT(al.getAlphabet() != NULL, ti.setError(MSFFormat::tr("Alphabet unknown")), );
    
    MAlignmentObject* obj = new MAlignmentObject(al);
    objects.append(obj);
}

Document* MSFFormat::loadDocument(IOAdapter* io, const U2DbiRef& dbiRef, const QVariantMap& fs, U2OpStatus& os){
    QList<GObject*> objs;
    load(io, objs, os);

    CHECK_OP_EXT(os, qDeleteAll(objs), NULL);
    return new Document(this, io->getFactory(), io->getURL(), dbiRef, objs, fs);
}

static bool writeBlock(IOAdapter *io, U2OpStatus& ti, const QByteArray& buf) {
    int len = io->writeBlock(buf);
    if (len != buf.length()) {
        ti.setError(L10N::errorTitle());
        return true;
    }
    return false;
}

void MSFFormat::storeDocument(Document* d, IOAdapter* io, U2OpStatus& os) {
    MAlignmentObject* obj = NULL;
    if((d->getObjects().size() != 1) || ((obj = qobject_cast<MAlignmentObject*>(d->getObjects().first())) == NULL)) {
        os.setError("No data to write;");
        return;
    }

    QList<GObject*> als; als << obj;
    QMap< GObjectType, QList<GObject*> > objectsMap;
    objectsMap[GObjectTypes::MULTIPLE_ALIGNMENT] = als;
    storeEntry(io, objectsMap, os);
    CHECK_EXT(!os.isCoR(), os.setError(L10N::errorWritingFile(d->getURL())), );
}

void MSFFormat::storeEntry(IOAdapter *io, const QMap< GObjectType, QList<GObject*> > &objectsMap, U2OpStatus &os) {
    SAFE_POINT(objectsMap.contains(GObjectTypes::MULTIPLE_ALIGNMENT), "MSF entry storing: no alignment", );
    const QList<GObject*> &als = objectsMap[GObjectTypes::MULTIPLE_ALIGNMENT];
    SAFE_POINT(1 == als.size(), "MSF entry storing: alignment objects count error", );

    const MAlignmentObject* obj = dynamic_cast<MAlignmentObject*>(als.first());
    SAFE_POINT(NULL != obj, "MSF entry storing: NULL alignment object", );

    const MAlignment& ma = obj->getMAlignment();

    //precalculate seq writing params
    int maxNameLen = 0, maLen = ma.getLength(), checkSum = 0;
    static int maxCheckSumLen = 4;
    QMap <QString, int> checkSums;
    foreach(const MAlignmentRow& row , ma.getRows()) {
        QByteArray sequence = row.toByteArray(maLen).replace(MAlignment_GapChar, '.');
        int seqCheckSum = getCheckSum(sequence);
        checkSums.insert(row.getName(), seqCheckSum);
        checkSum = (checkSum + seqCheckSum) % CHECK_SUM_MOD;
        maxNameLen = qMax(maxNameLen, row.getName().length());
    }
    int maxLengthLen = QString::number(maLen).length();

    //write first line
    QByteArray line = "  " + MSF_FIELD;
    line += " " + QByteArray::number(maLen);
    line += "  " + TYPE_FIELD;
    line += " " + ( obj->getAlphabet()->isAmino() ? TYPE_VALUE_PROTEIN : TYPE_VALUE_NUCLEIC );
    line += "  " + QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm");
    line += "  " + CHECK_FIELD;
    line += " " + QByteArray::number(checkSum);
    line += "  " + END_OF_HEADER_LINE + "\n\n";
    if (writeBlock(io, os, line))
        return;

    //write info
    foreach(const MAlignmentRow& row, ma.getRows()) {
        QByteArray line = " " + NAME_FIELD;
        line += " " + QString(row.getName()).replace(' ', '_').leftJustified(maxNameLen+1); // since ' ' is a delimeter for MSF parser spaces in name not suppoted
        line += "  " + LEN_FIELD;
        line += " " + QString("%1").arg(maLen, -maxLengthLen);
        line += "  " + CHECK_FIELD;
        line += " " + QString("%1").arg(checkSums[row.getName()], -maxCheckSumLen);
        line += "  " + WEIGHT_FIELD;
        line += " " + QByteArray::number(WEIGHT_VALUE) + "\n";
        if (writeBlock(io, os, line)) {
            return;
        }
    }
    if (writeBlock(io, os, "\n" + SECTION_SEPARATOR + "\n\n")) {
        return;
    }

    for (int i = 0; !os.isCoR() && i < maLen; i += CHARS_IN_ROW) {
        /* write numbers */ {
            QByteArray line(maxNameLen + 2, ' ');
            QString t = QString("%1").arg(i + 1);
            QString s = QString("%1").arg(i + CHARS_IN_ROW < maLen ? i + CHARS_IN_ROW : maLen);
            int r = maLen - i < CHARS_IN_ROW ? maLen % CHARS_IN_ROW : CHARS_IN_ROW;
            r += (r - 1) / CHARS_IN_WORD - (t.length() + s.length());
            line += t;
            if (r > 0) {
                line += QByteArray(r, ' ');
                line += s;
            }
            line += '\n';
            if (writeBlock(io, os, line)) {
                return;
            }
        }

        //write sequence
        foreach(const MAlignmentRow& row, ma.getRows()) {
            QByteArray line = row.getName().toLocal8Bit();
            line.replace(' ', '_'); // since ' ' is a delimiter for MSF parser spaces in name not supported
            line = line.leftJustified(maxNameLen+1);

            for (int j = 0; j < CHARS_IN_ROW && i + j < maLen; j += CHARS_IN_WORD) {
                line += ' ';
                int nChars = qMin(CHARS_IN_WORD, maLen - (i + j));
                line += row.mid(i + j, nChars).toByteArray(nChars).replace(MAlignment_GapChar, '.');
            }
            line += '\n';
            if (writeBlock(io, os, line)) {
                return;
            }
        }
        if (writeBlock(io, os, "\n")) {
            return;
        }
    }
}


FormatCheckResult MSFFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    const char* data = rawData.constData();
    int size = rawData.size();

    bool hasBinaryData = TextUtils::contains(TextUtils::BINARY, data, size);
    if (hasBinaryData) {
        return FormatDetection_NotMatched;
    }
    if (rawData.contains("MSF:") 
        || rawData.contains("!!AA_MULTIPLE_ALIGNMENT 1.0") || rawData.contains("!!NA_MULTIPLE_ALIGNMENT 1.0")
        || (rawData.contains("Name:") && rawData.contains("Len:") && rawData.contains("Check:") && rawData.contains("Weight:"))) 
    {
        return FormatDetection_VeryHighSimilarity;
    }

    if (rawData.contains("GDC ")) {
        return FormatDetection_AverageSimilarity;
    }

    //MSF documents may contain unlimited number of comment lines in header ->
    //it is impossible to determine if file has MSF format by some predefined
    //amount of raw data read from it.
    if (rawData.contains("GCG ") || rawData.contains("MSF ")) {
        return FormatDetection_LowSimilarity;
    }
    return FormatDetection_VeryLowSimilarity;
}

} //namespace U2
