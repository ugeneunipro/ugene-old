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

#include "MegaFormat.h"
#include <U2Formats/DocumentFormatUtils.h>
#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/TextUtils.h>
#include <U2Core/MSAUtils.h>

namespace U2 {

const QByteArray MegaFormat::MEGA_HEADER="mega";
const QByteArray MegaFormat::MEGA_UGENE_TITLE="!Title Ugene Mega;\n\n";
const QByteArray MegaFormat::MEGA_TITLE="TITLE";
const char MegaFormat::MEGA_SEPARATOR='#';
const char MegaFormat::MEGA_IDENTICAL='.';
const char MegaFormat::MEGA_INDEL='-';
const char MegaFormat::MEGA_START_COMMENT='!';
const char MegaFormat::MEGA_END_COMMENT=';';

MegaFormat::MegaFormat(QObject* p) : DocumentFormat(p, DocumentFormatFlags_SW, QStringList("meg")) {
    formatName = tr("Mega");
    supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
}

Document* MegaFormat::loadDocument(IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs, DocumentLoadMode) {
    QList <GObject*> objs;
    load(io, objs, ti);

    if (ti.hasError()) {
        qDeleteAll(objs);
        return NULL;
    }
    return new Document(this, io->getFactory(), io->getURL(), objs, fs);
}

void MegaFormat::storeDocument( Document* d, TaskStateInfo& ti, IOAdapter* io ) {
    if( NULL == d ) {
        ti.setError(L10N::badArgument("doc"));
        return;
    }
    if( NULL == io || !io->isOpen() ) {
        ti.setError(L10N::badArgument("IO adapter"));
        return;
    }
    save(io, d, ti);
}

FormatCheckResult MegaFormat::checkRawData(const QByteArray& rawData, const GUrl&) const {
    QByteArray line=rawData.trimmed();

    if (!line.startsWith(MEGA_SEPARATOR)) {
        return FormatDetection_NotMatched;
    }
    line=line.mid(1);
    line=line.trimmed();
    if (!line.startsWith(MEGA_HEADER)) {
        return FormatDetection_NotMatched;
    }
    return FormatDetection_Matched;
}

static bool checkModel(const MAlignment& al, TaskStateInfo& ti) {
    //check that all sequences are of equal size
    int size = 0;
    for (int i=0, n = al.getNumRows(); i<n; i++) {
        const MAlignmentRow& item = al.getRow(i);
        if (i == 0) {
            size = item.getCoreLength();//item.toByteArray().size();
        } else {
            int itemSize = item.getCoreLength();//item.toByteArray().size();
            if (size!=itemSize) {
                ti.setError( MegaFormat::tr("Found sequences of different sizes"));
                return false;
            }
        }
    }
    if (size == 0) {
        ti.setError( MegaFormat::tr("Model is of zero size"));
        return false;
    } 
    return true;
}

bool MegaFormat::getNextLine(IOAdapter* io, QByteArray& line) {
    line.clear();
    static int READ_BUFF_SIZE = 4096;
    QByteArray readBuffer(READ_BUFF_SIZE, '\0');
    char* buff = readBuffer.data();

    qint64 len;
    bool eolFound = false, eof = false;
    while (!eolFound) {
        len = io->readLine(buff, READ_BUFF_SIZE, &eolFound);
        if (len < READ_BUFF_SIZE && !eolFound)
            eolFound = eof = true;
        line += readBuffer;
    }

    if (len != READ_BUFF_SIZE)
        line.resize(line.size() + len - READ_BUFF_SIZE);

    return eof;
}

bool MegaFormat::checkName(QByteArray &name) {
    if (name.contains(MEGA_SEPARATOR) ||
        name.contains(MEGA_START_COMMENT) ||
        name.contains(MEGA_END_COMMENT)) {
        return false;
    }

    return true;
}

bool MegaFormat::readName(IOAdapter* io, QByteArray &line, QByteArray &name, TaskStateInfo &ti) {
    bool eof=false;

    line=line.mid(1);
    line=line.trimmed();
    skipWhites(io, line);
    if (line.isEmpty()) {
        return true;
    }
    line=line.simplified();

    int spaceIdx=line.indexOf(' ');
    if (-1!=spaceIdx) {
        name=line.left(spaceIdx);
        line=line.mid(spaceIdx);
    } else {
        name=line;
        eof=getNextLine(io, line);
        line=line.simplified();
    }
    if (!checkName(name)) {
        ti.setError(MegaFormat::tr("Bad name of sequence"));
    }

    ti.progress=io->getProgress();
    return eof;
}

bool MegaFormat::skipComments(IOAdapter *io, QByteArray &line, TaskStateInfo &ti) {
    int i=0;
    bool eof=false;
    bool hasEnd=false;

    while (1) {
        while (i<line.length() && !hasEnd) {
            if (MEGA_END_COMMENT==line[i]) {
                i++;
                hasEnd=true;
                break;
            }
            if (MEGA_SEPARATOR==line[i]) {
                ti.setError(MegaFormat::tr("Unexpected # in comments"));
                return eof;
            }
            i++;
        }
        if (line.length()==i) {
            if (eof) {
                line.clear();
                if (!hasEnd) {
                    ti.setError(MegaFormat::tr("A comment has not end"));
                    return eof;
                }
                break;
            }
            eof=getNextLine(io, line);
            line=line.simplified();
            i=0;
            if (!hasEnd) {
                continue;
            }
        }
        hasEnd=true;
        while (i<line.length()) {
            if (MEGA_START_COMMENT==line[i]) {
                hasEnd=false;
                break;
            } else if (MEGA_SEPARATOR==line[i]) {
                line=line.mid(i);
                i=-1;
                break;
            } else if (' '!=line[i]) {
                ti.setError(MegaFormat::tr("Unexpected symbol between comments"));
                return eof;
            }
            i++;
        }
        if (!hasEnd) {
            continue;
        }
        if (line.length()!=i) {
            break;
        }
        if (line.length()==i && eof) {
            line.clear();
            break;
        }
    }

    ti.progress = io->getProgress();
    return eof;
}

void MegaFormat::workUpIndels(MAlignment& al) {
    QByteArray firstSequence=al.getRow(0).getCore();

    for (int i=1; i<al.getNumRows(); i++) {
        QByteArray newSeq=al.getRow(i).getCore();
        for (int j=0; j<newSeq.length(); j++) {
            if (MEGA_IDENTICAL==al.charAt(i, j)) {
                newSeq[j]=firstSequence[j];
            }
        }
        al.setRowSequence(i, newSeq);
    }
}

void MegaFormat::load(U2::IOAdapter *io, QList<GObject*> &objects, U2::TaskStateInfo &ti) {
    MAlignment al(io->getURL().baseFileName());
    QByteArray line;
    bool eof=false;
    bool firstBlock=true;
    int sequenceIdx=0;
    bool lastIteration=false;

    readHeader(io, line, ti);
    if (ti.hasError()) {
        return;
    }
    readTitle(io, line, ti);
    if (ti.hasError()) {
        return;
    }

    //read data
    while (!ti.cancelFlag && !lastIteration) {
        QByteArray name;
        QByteArray value;

        //read name of a sequence
        if (readName(io, line, name, ti)) {
            if (!eof && name.isEmpty()) {
                ti.setError(MegaFormat::tr("Incorrect format"));
                return;
            } else if (name.isEmpty()) {
                break;
            }
        }
        if (ti.hasError()) {
            return;
        }

        //read the sequence
        eof=readSequence(io, line, ti, value, &lastIteration);
        if (ti.hasError()) {
            return;
        }
        if (0==sequenceIdx && value.contains(MEGA_IDENTICAL)) {
            ti.setError(MegaFormat::tr("Identical symbol at the first sequence"));
            return;
        }

        if (firstBlock) {
            for (int i=0; i<al.getNumRows(); i++) {
                if (al.getRow(i).getName()==name) {
                    firstBlock=false;
                    sequenceIdx=0;
                    break;
                }
            }
        }
        //add the sequence to the list
        if (firstBlock) {
            al.addRow(MAlignmentRow(name, value));
            sequenceIdx++;
        } else {
            if (sequenceIdx<al.getNumRows()) {
                const MAlignmentRow *item=&al.getRow(sequenceIdx);

                if (item->getName()!=name) {
                    ti.setError(MegaFormat::tr("Incorrect order of sequences' names"));
                    return;
                }
                al.appendChars(sequenceIdx, value.constData(), value.size());
            }
            sequenceIdx++;
            if (sequenceIdx==al.getNumRows()) {
                sequenceIdx=0;
            }
        }
    }

    checkModel(al, ti);
    if (ti.hasError()) {
        return;
    }

    DocumentFormatUtils::assignAlphabet(al);
    if (al.getAlphabet() == NULL) {
        ti.setError(MegaFormat::tr("Alphabet unknown"));
        return;
    }

    workUpIndels(al); //replace '.' by symbols from the first sequence

    MAlignmentObject* obj = new MAlignmentObject(al);
    objects.append(obj);
}

void MegaFormat::save(U2::IOAdapter *io, U2::Document *d, U2::TaskStateInfo &ti) {
    const MAlignmentObject* obj = NULL;
    if( (d->getObjects().size() != 1)
        || ((obj = qobject_cast<const MAlignmentObject*>(d->getObjects().first())) == NULL)) {
            ti.setError("No data to write;");
            return;
    }
    const MAlignment& ma = obj->getMAlignment();

    //write header
    QByteArray header;
    header.append(MEGA_SEPARATOR).append(MEGA_HEADER).append("\n").append(MEGA_UGENE_TITLE);
    int len = io->writeBlock(header);
    if (len != header.length()) {
        ti.setError(L10N::errorWritingFile(d->getURL()));
        return;
    }

    int maxNameLength=0;
    foreach (MAlignmentRow item, ma.getRows()) {
        maxNameLength = qMax(maxNameLength, item.getName().length());
    }

    //write data
    int seqLength=ma.getLength();
    int writtenLength=0;
    while (writtenLength<seqLength) {
        foreach (const MAlignmentRow & item, ma.getRows()) {
            QByteArray line;
            line.append(MEGA_SEPARATOR).append(item.getName());
            TextUtils::replace(line.data(), line.length(), TextUtils::WHITES, '_');
            
            for (int i=0; i<maxNameLength-item.getName().length()+1; i++) {
                line.append(' ');
            }
            
            QByteArray currentBlock=item.mid(writtenLength, BLOCK_LENGTH).
                toByteArray(writtenLength + BLOCK_LENGTH > seqLength ? seqLength - writtenLength : BLOCK_LENGTH);
            line.append(currentBlock).append('\n');
            
            len = io->writeBlock(line);
            if (len != line.length()) {
                ti.setError(L10N::errorWritingFile(d->getURL()));
                return;
            }
        }
        writtenLength+=BLOCK_LENGTH;
        
        io->writeBlock("\n\n");
    }
}

void MegaFormat::readHeader(U2::IOAdapter *io, QByteArray &line, U2::TaskStateInfo &ti) {
    skipWhites(io, line);
    if (line.isEmpty()) {
        ti.setError(MegaFormat::tr("No header"));
        return;
    }

    if (!line.startsWith(MEGA_SEPARATOR)) {
        ti.setError(MegaFormat::tr("No # before header"));
        return;
    }
    line=line.mid(1);
    line=line.trimmed();
    skipWhites(io, line);
    if (line.isEmpty()) {
        ti.setError(MegaFormat::tr("No header"));
        return;
    }

    if (!line.startsWith(MEGA_HEADER)) {
        ti.setError(MegaFormat::tr("Not MEGA-header"));
        return;
    }
    line=line.mid(MEGA_HEADER.length());
    line=line.trimmed();
    ti.progress = io->getProgress();
}

void MegaFormat::skipWhites(U2::IOAdapter *io, QByteArray &line) {
    while (line.isEmpty()) {
        if (getNextLine(io, line)) {
            if (line.isEmpty()) {
                return;
            }
        }

        line=line.trimmed();
    }
}

void MegaFormat::readTitle(U2::IOAdapter *io, QByteArray &line, U2::TaskStateInfo &ti) {
    skipWhites(io, line);
    if (line.isEmpty()) {
        ti.setError(MegaFormat::tr("No data in file"));
        return;
    }

    bool comment=false;
    if (MEGA_START_COMMENT==line[0]) {
        line=line.mid(1);
        line=line.trimmed();
        comment=true;
        skipWhites(io, line);
        if (line.isEmpty()) {
            ti.setError(MegaFormat::tr("No data in file"));
            return;
        }
    }

    line=line.simplified();
    QByteArray word=line.left(MEGA_TITLE.length());
    word=word.toUpper();
    if (MEGA_TITLE!=word) {
        ti.setError(MegaFormat::tr("Incorrect title"));
        return;
    }
    line=line.mid(MEGA_TITLE.length());
    if (!line.isEmpty() &&
        (TextUtils::ALPHA_NUMS[line[0]] || MEGA_IDENTICAL==line[0] || MEGA_INDEL==line[0])){
        ti.setError(MegaFormat::tr("Incorrect title"));
        return;
    }

    //read until #
    if (comment) {
        skipComments(io, line, ti);
    } else {
        int sepIdx=line.indexOf(MEGA_SEPARATOR);
        while (-1==sepIdx) {
            if (getNextLine(io, line)) {
                if (line.isEmpty()) {
                    ti.setError(MegaFormat::tr("No data in file"));
                    return;
                }
            }
            sepIdx=line.indexOf(MEGA_SEPARATOR);
        }
        line=line.mid(sepIdx);
    }
    ti.progress = io->getProgress();
}

bool MegaFormat::readSequence(U2::IOAdapter *io, QByteArray &line, U2::TaskStateInfo &ti, 
                              QByteArray &value, bool *lastIteration) {
    bool hasPartOfSequence=false;
    bool eof=false;
    while (!ti.cancelFlag) {
        //delete spaces from the sequence untill #
        int spaceIdx=line.indexOf(' ');
        int separatorIdx;
        while (-1!=spaceIdx) {
            separatorIdx=line.indexOf(MEGA_SEPARATOR);
            if (-1!=separatorIdx && separatorIdx<spaceIdx) {
                break;
            }
            line=line.left(spaceIdx).append(line.mid(spaceIdx+1));
            spaceIdx=line.indexOf(' ');
        }

        //read another part if it is needed
        if (line.isEmpty()) {
            if (getNextLine(io, line)) {
                if (!hasPartOfSequence) {
                    ti.setError(MegaFormat::tr("Sequence has empty part"));
                    return eof;
                } else {
                    eof=true;
                    break;
                }
            }
            ti.progress = io->getProgress();
            line=line.simplified();
            continue;
        }

        separatorIdx=line.indexOf(MEGA_SEPARATOR);
        int commentIdx=line.indexOf(MEGA_START_COMMENT);

        int sequenceEnd=(-1==separatorIdx)?line.size():separatorIdx;
        sequenceEnd=(-1==commentIdx)?sequenceEnd:qMin(sequenceEnd, commentIdx);
        //check symbols in the sequence
        for (int i=0; i<sequenceEnd; i++) {
            if (!(TextUtils::ALPHAS[line[i]]) && !(line[i]==MEGA_INDEL) && !(line[i]==MEGA_IDENTICAL)) {
                ti.setError(MegaFormat::tr("Bad symbols in a sequence"));
                return eof;
            }
        }
        value.append(line, sequenceEnd);
        hasPartOfSequence=true;

        if (-1!=commentIdx) { //skip comments untill #
            if ((-1!=separatorIdx && commentIdx<separatorIdx)
             || -1==separatorIdx) {
                line=line.mid(commentIdx);
                eof=skipComments(io, line, ti);
                if (ti.hasError()) {
                    return eof;
                }
                line=line.simplified();
                if (!line.isEmpty()) {
                    separatorIdx=0;
                }
            }
        }
        if (eof) {
            (*lastIteration)=true;
            break;
        }
        if (-1==separatorIdx) {
            if (getNextLine(io, line)) {
                if (!line.isEmpty()) {
                    ti.progress = io->getProgress();
                    line=line.simplified();
                    continue;
                }
                eof=true;
                break;
            }
            ti.progress = io->getProgress();
            line=line.simplified();
            continue;
        } else {
            line=line.mid(separatorIdx);
            break;
        }
    }

    return eof;
}
} //namespace
