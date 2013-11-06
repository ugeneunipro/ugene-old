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

#include "PhylipFormat.h"

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/L10n.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2AlphabetUtils.h>

#include <U2Algorithm/MSAConsensusUtils.h>
#include <U2Algorithm/MSAConsensusAlgorithmRegistry.h>
#include <U2Algorithm/BuiltInConsensusAlgorithms.h>


namespace U2 {

// PhylipFormat
PhylipFormat::PhylipFormat(QObject *p)
    : DocumentFormat(p, DocumentFormatFlags(DocumentFormatFlag_SupportWriting) | DocumentFormatFlag_OnlyOneObject,
                     QStringList() << "phy" << "ph"){
    formatDescription = tr("PHYLIP multiple alignment format for phylogenetic applications.");
    supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
}

void PhylipFormat::storeDocument(Document *d, IOAdapter *io, U2OpStatus &os) {
    CHECK_EXT(d!=NULL, os.setError(L10N::badArgument("doc")), );
    CHECK_EXT(io != NULL && io->isOpen(), os.setError(L10N::badArgument("IO adapter")), );

    MAlignmentObject *obj = NULL;
    CHECK_EXT(d->getObjects().size() == 1, os.setError("Incorrect number of objects in document"), );
    CHECK_EXT((obj = qobject_cast<MAlignmentObject*>(d->getObjects().first())) != NULL, os.setError("No data to write"), );

    QList<GObject*> als;
    als << obj;
    QMap< GObjectType, QList<GObject*> > objectsMap;
    objectsMap[GObjectTypes::MULTIPLE_ALIGNMENT] = als;
    storeEntry(io, objectsMap, os);
    CHECK_EXT(!os.isCoR(), os.setError(L10N::errorWritingFile(d->getURL())), );
}

MAlignmentObject* PhylipFormat::load(IOAdapter *io, const U2DbiRef &dbiRef, U2OpStatus &os) {
    SAFE_POINT(io != NULL, "IO adapter is NULL!", NULL);

    MAlignment al = parse(io, os);
    CHECK_OP(os, NULL);
    MSAUtils::checkPackedModelSymmetry(al, os);
    CHECK_OP(os, NULL);

    U2AlphabetUtils::assignAlphabet(al);
    CHECK_EXT(al.getAlphabet()!=NULL, os.setError( PhylipFormat::tr("Alphabet is unknown")), NULL);

    U2EntityRef msaRef = MAlignmentImporter::createAlignment(dbiRef, al, os);
    CHECK_OP(os, NULL);

    MAlignmentObject* obj = new MAlignmentObject(al.getName(), msaRef);
    return obj;
}

bool PhylipFormat::parseHeader(QByteArray data, int &species, int &characters) const {
    QTextStream stream(data);
    stream >> species >> characters;
    if ((species == 0) && (characters == 0)) {
        return false;
    }
    return true;
}

void PhylipFormat::removeSpaces(QByteArray &data) const {
    while (data.contains(' ')) {
        data.remove(data.indexOf(' '), 1);
    }
}

Document* PhylipFormat::loadDocument(IOAdapter *io, const U2DbiRef &dbiRef, const QVariantMap &fs, U2OpStatus &os) {
    SAFE_POINT(io != NULL, "IO adapter is NULL!", NULL);
    QList<GObject*> objects;
    objects.append( load(io, dbiRef, os) );
    CHECK_OP_EXT(os, qDeleteAll(objects), NULL);
    assert(objects.size() == 1);
    return new Document(this, io->getFactory(), io->getURL(), dbiRef, objects, fs);
}


#define READ_BUFF_SIZE  1024
#define MAX_NAME_LEN    10  // max name length for phylip format is 10

#define SEQ_BLOCK_SIZE  100
#define INT_BLOCK_SIZE  50

const QBitArray& LINE_BREAKS = TextUtils::LINE_BREAKS;

// PhylipSequentialFormat
PhylipSequentialFormat::PhylipSequentialFormat(QObject *p)
    : PhylipFormat(p) {
    formatName = tr("PHYLIP Sequantial");
}

void PhylipSequentialFormat::storeEntry(IOAdapter *io, const QMap<GObjectType, QList<GObject *> > &objectsMap, U2OpStatus &os) {
    SAFE_POINT(io != NULL, "IO adapter is NULL!", );
    SAFE_POINT(objectsMap.contains(GObjectTypes::MULTIPLE_ALIGNMENT), "PHYLIP entry storing: no alignment", );
    const QList<GObject*> &als = objectsMap[GObjectTypes::MULTIPLE_ALIGNMENT];
    SAFE_POINT(1 == als.size(), "PHYLIP entry storing: alignment objects count error", );

    const MAlignmentObject* obj = dynamic_cast<MAlignmentObject*>(als.first());
    SAFE_POINT(NULL != obj, "PHYLIP entry storing: NULL alignment object", );

    const MAlignment& ma = obj->getMAlignment();

    //write header
    int numberOfSpecies = ma.getNumRows();
    int numberOfCharacters = ma.getLength();
    QByteArray header( (QString::number(numberOfSpecies) + " " + QString::number(numberOfCharacters)).toLatin1() + "\n");
    int len = io->writeBlock(header);
    CHECK_EXT(len == header.length(), os.setError(L10N::errorTitle()), );

    //write sequences
    for (int i = 0; i < numberOfSpecies; i++) {
        QByteArray line = ma.getRow(i).getName().toLatin1();
        if (line.length() < MAX_NAME_LEN) {
            int difference = MAX_NAME_LEN - line.length();
            for (int j = 0; j < difference; j++) {
                line.append(" ");
            }
        }
        if (line.length() > MAX_NAME_LEN) {
            line = line.left(MAX_NAME_LEN);
        }
        io->writeBlock(line);
        QByteArray sequence = ma.getRow(i).toByteArray(numberOfCharacters, os);
        int blockCounter = 0;
        while ((blockCounter*SEQ_BLOCK_SIZE) <= numberOfCharacters) {
            line.clear();
            line.append(sequence.mid(blockCounter*SEQ_BLOCK_SIZE, SEQ_BLOCK_SIZE));
            line.append('\n');
            io->writeBlock(line);
            blockCounter++;
        }
    }
}

FormatCheckResult PhylipSequentialFormat::checkRawData(const QByteArray &rawData, const GUrl &) const {
    if (TextUtils::contains(TextUtils::BINARY, rawData.constData(), rawData.size())) {
        return FormatDetection_NotMatched;
    }
    int species = 0, characters = 0;
    if (!parseHeader(rawData, species, characters)) {
        return FormatDetection_NotMatched;
    }
    QTextStream s(rawData);
    for (int i = 0; i  < species + 1; i++) {
        if (s.atEnd()) {
            return FormatDetection_AverageSimilarity;
        }
        s.readLine();
    }
    // if line after row names is not empty and contains characters at the beginning,
    // it is more probably a sequential phylip example
    QString line = s.readLine();
    if ((line.size() != 0) && (line.at(0) != ' ')) {
        return FormatDetection_Matched;
    }

    return FormatDetection_AverageSimilarity;
}

MAlignment PhylipSequentialFormat::parse(IOAdapter *io, U2OpStatus &os) const {
    SAFE_POINT(io != NULL, "IO adapter is NULL!", MAlignment());
    QByteArray readBuffer(READ_BUFF_SIZE, '\0');
    char* buff = readBuffer.data();
    QString objName = io->getURL().baseFileName();
    MAlignment al(objName);
    bool resOk = false;

    // Header: "<number of species> <number of characters>"
    int len = io->readLine(buff, READ_BUFF_SIZE, &resOk);
    CHECK_EXT(resOk, os.setError( PhylipSequentialFormat::tr("Illegal line")), MAlignment());

    QByteArray line = QByteArray(buff, len).trimmed();

    int numberOfSpecies = 0;
    int numberOfCharacters = 0;
    resOk = parseHeader(line, numberOfSpecies, numberOfCharacters);
    CHECK_EXT(resOk, os.setError( PhylipSequentialFormat::tr("Wrong header") ), MAlignment());

    for (int i = 0; i < numberOfSpecies; i++) {
        CHECK_EXT(!io->isEof(), os.setError( PhylipSequentialFormat::tr("There is not enough data")), MAlignment());
        // get name
        len = io->readBlock(buff, MAX_NAME_LEN);
        QByteArray name;
        name.append(QByteArray::fromRawData(buff, len).trimmed());
        CHECK_EXT(len != 0, os.setError( PhylipSequentialFormat::tr("Error parsing file") ), MAlignment());

        // get sequence
        QByteArray value;
        while ((value.size() != numberOfCharacters) && (!io->isEof())) {
            len = io->readUntil(buff, READ_BUFF_SIZE, LINE_BREAKS, IOAdapter::Term_Skip, &resOk);
            QByteArray line = QByteArray::fromRawData(buff, len);
            removeSpaces(line);
            value.append(line);
        }
        al.addRow(name, value, os);

        os.setProgress(io->getProgress());
    }
    CHECK_EXT(al.getLength() == numberOfCharacters, os.setError( PhylipSequentialFormat::tr("Number of characters does not correspond to the stated number") ),
               MAlignment());
    return al;
}


// PhylipInterleavedFormat
PhylipInterleavedFormat::PhylipInterleavedFormat(QObject *p)
    :PhylipFormat(p) {
    formatName = tr("PHYLIP Interleaved");
}

void PhylipInterleavedFormat::storeEntry(IOAdapter *io, const QMap<GObjectType, QList<GObject *> > &objectsMap, U2OpStatus &os) {
    SAFE_POINT(io != NULL, "IO adapter is NULL!", );
    SAFE_POINT(objectsMap.contains(GObjectTypes::MULTIPLE_ALIGNMENT), "PHYLIP entry storing: no alignment", );
    const QList<GObject*> &als = objectsMap[GObjectTypes::MULTIPLE_ALIGNMENT];
    SAFE_POINT(1 == als.size(), "PHYLIP entry storing: alignment objects count error", );

    const MAlignmentObject* obj = dynamic_cast<MAlignmentObject*>(als.first());
    SAFE_POINT(NULL != obj, "PHYLIP entry storing: NULL alignment object", );

    const MAlignment& ma = obj->getMAlignment();

    //write header
    int numberOfSpecies = ma.getNumRows();
    int numberOfCharacters = ma.getLength();
    QByteArray header( (QString::number(numberOfSpecies) + " " + QString::number(numberOfCharacters)).toLatin1() + "\n");
    int len = io->writeBlock(header);

    CHECK_EXT(len == header.length(), os.setError(L10N::errorTitle()), );

    //write first block with names
    for (int i = 0; i < numberOfSpecies; i++) {
        QByteArray line = ma.getRow(i).getName().toLatin1();
        if (line.length() < MAX_NAME_LEN) {
            int difference = MAX_NAME_LEN - line.length();
            for (int j = 0; j < difference; j++)
                line.append(" ");
        }
        if (line.length() > MAX_NAME_LEN) {
            line = line.left(MAX_NAME_LEN);
        }

        QByteArray sequence = ma.getRow(i).toByteArray(numberOfCharacters, os);
        line.append(sequence.left(INT_BLOCK_SIZE));
        line.append('\n');

        io->writeBlock(line);
    }

    //write sequence blockss
    int blockCounter = 1;
    QByteArray spacer(MAX_NAME_LEN, ' ');
    while (blockCounter*INT_BLOCK_SIZE <= numberOfCharacters) {
        io->writeBlock("\n", 1);
        for (int i = 0; i < numberOfSpecies; i++) {
            QByteArray sequence = ma.getRow(i).toByteArray(numberOfCharacters, os);
            QByteArray line;
            line.append(spacer);
            line.append(sequence.mid(blockCounter*INT_BLOCK_SIZE, INT_BLOCK_SIZE));
            line.append('\n');

            io->writeBlock(line, line.size());
        }
        blockCounter++;
    }
}

FormatCheckResult PhylipInterleavedFormat::checkRawData(const QByteArray &rawData, const GUrl &) const {
    if (TextUtils::contains(TextUtils::BINARY, rawData.constData(), rawData.size())) {
        return FormatDetection_NotMatched;
    }
    int species, characters;
    if (!parseHeader(rawData, species, characters)) {
        return FormatDetection_NotMatched;
    }

    QTextStream s(rawData);
    for (int i = 0; i  < species + 1; i++) {
        if (s.atEnd()) {
            return FormatDetection_AverageSimilarity;
        }
        s.readLine();
    }
    // if line after row names is empty or contains spaces at the beginning,
    // it is more probably an interleaved phylip example
    QString line = s.readLine();
    if (((line.size() != 0) && (line.at(0) == ' '))
            || (line.isEmpty())) {
        return FormatDetection_Matched;
    }

    return FormatDetection_AverageSimilarity;
}

MAlignment PhylipInterleavedFormat::parse(IOAdapter *io, U2OpStatus &os) const {
    SAFE_POINT(io != NULL, "IO adapter is NULL!", MAlignment());

    QByteArray readBuffer(READ_BUFF_SIZE, '\0');
    char* buff = readBuffer.data();
    QString objName = io->getURL().baseFileName();
    MAlignment al(objName);

    bool resOk = false;

    // First line: "<number of species> <number of characters>"
    int len = io->readLine(buff, READ_BUFF_SIZE, &resOk);

    CHECK_EXT(resOk, os.setError( PhylipInterleavedFormat::tr("Illegal line") ), MAlignment());

    QByteArray line = QByteArray(buff, len).trimmed();

    int numberOfSpecies;
    int numberOfCharacters;
    resOk = parseHeader(line, numberOfSpecies, numberOfCharacters);
    CHECK_EXT(resOk,  os.setError( PhylipInterleavedFormat::tr("Wrong header") ), MAlignment());

    //the first block with the names
    for (int i = 0; i < numberOfSpecies; i++) {
        CHECK_EXT(!io->isEof(), os.setError( PhylipSequentialFormat::tr("There is not enough data")), MAlignment());
        len = io->readBlock(buff, MAX_NAME_LEN);
        CHECK_EXT(len != 0, os.setError( PhylipFormat::tr("Error parsing file") ), MAlignment());

        QByteArray name;
        name.append(QByteArray::fromRawData(buff, len).trimmed());

        len = io->readUntil(buff, READ_BUFF_SIZE, LINE_BREAKS, IOAdapter::Term_Skip, &resOk);
        CHECK_EXT(len != 0, os.setError( PhylipSequentialFormat::tr("Error parsing file") ), MAlignment());

        QByteArray value = QByteArray::fromRawData(buff, len);
        removeSpaces(value);
        al.addRow(name, value, os);

        os.setProgress(io->getProgress());
    }

    // sequence blocks
    while (!os.isCoR() && len > 0 && !io->isEof()) {
        for (int i = 0; i < numberOfSpecies; i++) {
            len = io->readUntil(buff, READ_BUFF_SIZE, LINE_BREAKS, IOAdapter::Term_Skip, &resOk);
            if (len == 0) {
                    break;
            }
            QByteArray value = QByteArray::fromRawData(buff, len);
            removeSpaces(value);

            al.appendChars(i, value.constData(), value.size());
        }
        os.setProgress(io->getProgress());
    }
    CHECK_EXT(al.getLength() == numberOfCharacters, os.setError( PhylipInterleavedFormat::tr("Number of characters does not correspond to the stated number") ),
              MAlignment());
    return al;
}

} //namespace
