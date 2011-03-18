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

#include "BAMDbiPlugin.h"
#include "InvalidFormatException.h"
#include "Reader.h"

namespace U2 {
namespace BAM {

Reader::Reader(IOAdapter &ioAdapter):
        ioAdapter(ioAdapter),
        reader(ioAdapter)
{
    readHeader();
}

const Header &Reader::getHeader()const {
    return header;
}

Alignment Reader::readAlignment() {
    Alignment alignment;
    int blockSize = readInt32();
    if(blockSize < 0) {
        throw InvalidFormatException(BAMDbiPlugin::tr("Invalid block size: %1").arg(blockSize));
    }
    {
        int referenceId = readInt32();
        if((referenceId < -1) || (referenceId >= header.getReferences().size())) {
            throw InvalidFormatException(BAMDbiPlugin::tr("Invalid reference id: %1").arg(referenceId));
        }
        alignment.setReferenceId(referenceId);
    }
    {
        int position = readInt32();
        if(position < -1) {
            throw InvalidFormatException(BAMDbiPlugin::tr("Invalid read position: %1").arg(position));
        }
        alignment.setPosition(position);
    }
    int nameLength = 0;
    {
        quint32 value = readUint32();
        alignment.setBin(value >> 16);
        alignment.setMapQuality((value >> 8) & 0xff);
        nameLength = value & 0xff;
    }
    int cigarLength = 0;
    {
        quint32 value = readUint32();
        {
            Alignment::Flags flags;
            int flagsValue = value >> 16;
            if(flagsValue & 0x1) {
                flags |= Alignment::Fragmented;
            }
            if(flagsValue & 0x2) {
                flags |= Alignment::FragmentsAligned;
            }
            if(flagsValue & 0x4) {
                flags |= Alignment::Unmapped;
            }
            if(flagsValue & 0x8) {
                flags |= Alignment::NextUnmapped;
            }
            if(flagsValue & 0x10) {
                flags |= Alignment::Reverse;
            }
            if(flagsValue & 0x20) {
                flags |= Alignment::NextReverse;
            }
            if(flagsValue & 0x40) {
                flags |= Alignment::FirstInTemplate;
            }
            if(flagsValue & 0x80) {
                flags |= Alignment::LastInTemplate;
            }
            if(flagsValue & 0x100) {
                flags |= Alignment::SecondaryAlignment;
            }
            if(flagsValue & 0x200) {
                flags |= Alignment::FailsChecks;
            }
            if(flagsValue & 0x400) {
                flags |= Alignment::Duplicate;
            }
            alignment.setFlags(flags);
        }
        cigarLength = value & 0xffff;
    }
    int length = readInt32();
    if(length < 0) {
        throw InvalidFormatException(BAMDbiPlugin::tr("Invalid read length: %1").arg(length));
    }
    {
        int nextReferenceId = readInt32();
        if((nextReferenceId < -1) || (nextReferenceId >= header.getReferences().size())) {
            throw InvalidFormatException(BAMDbiPlugin::tr("Invalid mate reference id: %1").arg(nextReferenceId));
        }
        alignment.setNextReferenceId(nextReferenceId);
    }
    {
        int nextPosition = readInt32();
        if(nextPosition < -1) {
            throw InvalidFormatException(BAMDbiPlugin::tr("Invalid mate position: %1").arg(nextPosition));
        }
        alignment.setNextPosition(nextPosition);
    }
    {
        int templateLength = readInt32();
        if(!(alignment.getFlags() & Alignment::Fragmented)) {
            if(0 != templateLength) {
                throw InvalidFormatException(BAMDbiPlugin::tr("Invalid template length of a sigle-fragment template: %1").arg(templateLength));
            }
        }
        alignment.setTemplateLength(templateLength);
    }
    alignment.setName(readBytes(nameLength - 1));
    readChar();
    {
        QList<Alignment::CigarOperation> cigar;
        for(int index = 0;index < cigarLength;index++) {
            quint32 value = readUint32();
            Alignment::CigarOperation::Operation operation;
            switch(value & 0xf) {
            case 0:
                operation = Alignment::CigarOperation::AlignmentMatch;
                break;
            case 1:
                operation = Alignment::CigarOperation::Insertion;
                break;
            case 2:
                operation = Alignment::CigarOperation::Deletion;
                break;
            case 3:
                operation = Alignment::CigarOperation::Skipped;
                break;
            case 4:
                operation = Alignment::CigarOperation::SoftClip;
                break;
            case 5:
                operation = Alignment::CigarOperation::HardClip;
                break;
            case 6:
                operation = Alignment::CigarOperation::Padding;
                break;
            case 7:
                operation = Alignment::CigarOperation::SequenceMatch;
                break;
            case 8:
                operation = Alignment::CigarOperation::SequenceMismatch;
                break;
            default:
                throw InvalidFormatException(BAMDbiPlugin::tr("Invalid cigar operation code: %1").arg(value & 0xf));
            }
            int operatonLength = value >> 4;
            cigar.append(Alignment::CigarOperation(operatonLength, operation));
        }
        {
            int totalLength = 0;
            for(int index = 0;index < cigarLength;index++) {
                if(Alignment::CigarOperation::HardClip == cigar[index].getOperation()) {
                    if((index > 0) && (index < cigarLength - 1)) {
                        throw InvalidFormatException(BAMDbiPlugin::tr("Hard clip in the middle of cigar"));
                    }
                }
                if(Alignment::CigarOperation::SoftClip == cigar[index].getOperation()) {
                    if((index > 1) && (index < cigarLength - 2)) {
                        throw InvalidFormatException(BAMDbiPlugin::tr("Misplaced soft clip in the cigar"));
                    }
                    if((1 == index) && (Alignment::CigarOperation::HardClip != cigar[0].getOperation())) {
                        throw InvalidFormatException(BAMDbiPlugin::tr("Misplaced soft clip in the cigar"));
                    }
                    if((cigarLength - 2 == index) && (Alignment::CigarOperation::HardClip != cigar[cigarLength - 1].getOperation())) {
                        throw InvalidFormatException(BAMDbiPlugin::tr("Misplaced soft clip in the cigar"));
                    }
                }
                if((Alignment::CigarOperation::AlignmentMatch == cigar[index].getOperation()) ||
                   (Alignment::CigarOperation::Insertion == cigar[index].getOperation()) ||
                   (Alignment::CigarOperation::SoftClip == cigar[index].getOperation()) ||
                   (Alignment::CigarOperation::SequenceMatch == cigar[index].getOperation()) ||
                   (Alignment::CigarOperation::SequenceMismatch == cigar[index].getOperation())) {
                    totalLength += cigar[index].getLength();
                }
            }
            if(length != totalLength) {
                throw InvalidFormatException(BAMDbiPlugin::tr("Cigar length mismatch"));
            }
        }
        alignment.setCigar(cigar);
    }
    {
        QByteArray sequence(length, '\0');
        QByteArray packedSequence = readBytes((length + 1)/2);
        for(int index = 0;index < length;index++) {
            int value = 0;
            if(0 == (index%2)) {
                value = (packedSequence[index/2] >> 4) & 0xf;
            } else {
                value = packedSequence[index/2] & 0xf;
            }
            switch(value) {
            case 0:
                sequence[index] = '=';
                break;
            case 1:
                sequence[index] = 'A';
                break;
            case 2:
                sequence[index] = 'C';
                break;
            case 4:
                sequence[index] = 'G';
                break;
            case 8:
                sequence[index] = 'T';
                break;
            case 15:
                sequence[index] = 'N';
                break;
            default:
                throw InvalidFormatException(BAMDbiPlugin::tr("Invalid sequence character code: %1").arg(value));
            }
        }
        alignment.setSequence(sequence);
    }
    if(length > 0) {
        QByteArray quality = readBytes(length);
        bool hasQuality = false;
        for(int index = 0;index < quality.size();index++) {
            if(0xff != (unsigned char)quality[index]) {
                hasQuality = true;
                break;
            }
        }
        if(hasQuality) {
            alignment.setQuality(quality);
        }
    }
    {
        QMap<QByteArray, QVariant> optionalFields;
        int toRead = blockSize - 32 - nameLength - 4*cigarLength - (length + 1)/2 - length;
        int bytesRead = 0;
        while(bytesRead < toRead) {
            QByteArray tag = readBytes(2);
            foreach(QChar character, tag) {
                if(!character.isLetter()) {
                    throw InvalidFormatException(BAMDbiPlugin::tr("Invalid optional field tag: %1").arg(QString(tag)));
                }
            }
            char type = readChar();
            bytesRead += 3;
            QVariant value;
            switch(type) {
            case 'A':
                value = readChar();
                bytesRead += 1;
                break;
            case 'c':
                value = (int)readInt8();
                bytesRead += 1;
                break;
            case 'C':
                value = (int)readUint8();
                bytesRead += 1;
                break;
            case 's':
                value = (int)readInt16();
                bytesRead += 2;
                break;
            case 'S':
                value = (int)readUint16();
                bytesRead += 2;
                break;
            case 'i':
                value = (int)readInt32();
                bytesRead += 4;
                break;
            case 'I':
                value = (int)readUint32();
                bytesRead += 4;
                break;
            case 'f':
                value = readFloat32();
                bytesRead += 4;
                break;
            case 'Z':
                {
                    QByteArray string = readString();
                    value = string;
                    bytesRead += string.size() + 1;
                    break;
                }
            case 'H':
                {
                    QByteArray hexString = readString();
                    if(0 != (hexString.size()%2)) {
                        throw InvalidFormatException(BAMDbiPlugin::tr("Odd hex string length: %1").arg(hexString.size()));
                    }
                    QByteArray data(hexString.size()/2, 0);
                    for(int index = 0;index < hexString.size();index++) {
                        int digitValue = QChar(hexString[index]).digitValue();
                        if((-1 == digitValue) || (digitValue > 0xf)) {
                            throw InvalidFormatException(BAMDbiPlugin::tr("Invalid hex string digit: %1").arg(hexString[index]));
                        }
                        if(0 == (index%2)) {
                            data[index/2] = data[index/2] | (digitValue << 4);
                        } else {
                            data[index/2] = data[index/2] | digitValue;
                        }
                    }
                    value = data;
                    bytesRead += hexString.size() + 1;
                    break;
                }
            default:
                throw InvalidFormatException(BAMDbiPlugin::tr("Invalid optional field value type: %1").arg(type));
            }
            optionalFields.insert(tag, value);
        }
        if(bytesRead > toRead) {
            throw InvalidFormatException(BAMDbiPlugin::tr("Invalid block size: %1").arg(blockSize));
        }
        alignment.setOptionalFields(optionalFields);
    }
    return alignment;
}

bool Reader::isEof()const {
    return reader.isEof();
}

VirtualOffset Reader::getOffset()const {
    return reader.getOffset();
}

void Reader::seek(VirtualOffset offset) {
    reader.seek(offset);
}

void Reader::readBytes(char *buffer, qint64 size) {
    if(reader.read(buffer, size) < size) {
        throw InvalidFormatException(BAMDbiPlugin::tr("Unexpected end of file"));
    }
}

QByteArray Reader::readBytes(qint64 size) {
    QByteArray result(size, 0);
    readBytes(result.data(), result.size());
    return result;
}

qint32 Reader::readInt32() {
    char buffer[4];
    readBytes(buffer, sizeof(buffer));
    return (buffer[0] & 0xff) |
            ((buffer[1] & 0xff) << 8) |
            ((buffer[2] & 0xff) << 16) |
            (buffer[3] << 24);
}

quint32 Reader::readUint32() {
    char buffer[4];
    readBytes(buffer, sizeof(buffer));
    return (buffer[0] & 0xff) |
            ((buffer[1] & 0xff) << 8) |
            ((buffer[2] & 0xff) << 16) |
            ((buffer[3] & 0xff) << 24);
}

qint16 Reader::readInt16() {
    char buffer[2];
    readBytes(buffer, sizeof(buffer));
    return (buffer[0] & 0xff) |
            (buffer[1] << 8);
}

quint16 Reader::readUint16() {
    char buffer[2];
    readBytes(buffer, sizeof(buffer));
    return (buffer[0] & 0xff) |
            ((buffer[1] & 0xff) << 8);
}

qint8 Reader::readInt8() {
    char buffer[1];
    readBytes(buffer, sizeof(buffer));
    return buffer[0];
}

quint8 Reader::readUint8() {
    char buffer[1];
    readBytes(buffer, sizeof(buffer));
    return (buffer[0] & 0xff);
}

float Reader::readFloat32() {
    quint32 bits = readUint32();
    float *pointer = (float *)&bits;
    return *pointer;
}

char Reader::readChar() {
    char character = '\0';
    readBytes(&character, 1);
    return character;
}

QByteArray Reader::readString() {
    QByteArray result;
    while(true) {
        char character = readChar();
        if('\0' != character) {
            result.append(character);
        } else {
            break;
        }
    }
    return result;
}

void Reader::readHeader() {
    {
        QByteArray magic = readBytes(4);
        if("BAM\001" != magic) {
            throw InvalidFormatException(BAMDbiPlugin::tr("Invalid magic number"));
        }
    }
    QByteArray text;
    {
        int textSize = readInt32();
        if(textSize < 0) {
            throw InvalidFormatException(BAMDbiPlugin::tr("Invalid header text size: %1").arg(textSize));
        }
        text = readBytes(textSize);
    }
    QList<Header::Reference> references;
    {
        int referencesNumber = readInt32();
        if(referencesNumber < 0) {
            throw InvalidFormatException(BAMDbiPlugin::tr("Invalid number of references: %1").arg(referencesNumber));
        }
        for(int index = 0;index < referencesNumber;index++) {
            QByteArray name;
            {
                int nameLength = readInt32();
                if(nameLength < 0) {
                    throw InvalidFormatException(BAMDbiPlugin::tr("Invalid reference name length: %1").arg(nameLength));
                }
                name = readBytes(nameLength - 1);
                readChar();
                if(!QRegExp("[ -)+-<>-~][ -~]*").exactMatch(name)) {
                    throw InvalidFormatException(BAMDbiPlugin::tr("Invalid reference name: %1").arg(QString(name)));
                }
            }
            int length = readInt32();
            if(length < 0) {
                throw InvalidFormatException(BAMDbiPlugin::tr("Invalid reference sequence length: %1").arg(length));
            }
            referencesMap.insert(name, references.size());
            references.append(Header::Reference(name, length));
        }
    }
    {
        QList<Header::ReadGroup> readGroups;
        QList<Header::Program> programs;
        QList<QByteArray> previousProgramIds;
        foreach(const QByteArray &line, text.split('\n')) {
            if(line.isEmpty()) {
                continue;
            }
            if(line.startsWith("@CO")) {
                continue;
            }
            if(!line.startsWith('@')) {
                throw InvalidFormatException(BAMDbiPlugin::tr("Invalid header line: %1").arg(QString(line)));
            }
            QByteArray recordTag;
            QHash<QByteArray, QByteArray> fields;
            {
                QList<QByteArray> tokens = line.split('\t');
                recordTag = tokens[0].mid(1);
                if(!QRegExp("[A-Za-z][A-Za-z]").exactMatch(recordTag)) {
                    throw InvalidFormatException(BAMDbiPlugin::tr("Invalid header record tag: %1").arg(QString(recordTag)));
                }
                for(int index = 1;index < tokens.size();index++) {
                    QByteArray fieldTag;
                    QByteArray fieldValue;
                    {
                        int colonIndex = tokens[index].indexOf(':');
                        if(-1 != colonIndex) {
                            fieldTag = tokens[index].mid(0, colonIndex);
                            fieldValue = tokens[index].mid(colonIndex + 1);
                        } else if("PG" == recordTag) { // workaround for invalid headers produced by some programs
                            fieldTag = "ID";
                            fieldValue = tokens[index];
                        } else {
                            throw InvalidFormatException(BAMDbiPlugin::tr("Invalid header field: %1").arg(QString(tokens[index])));
                        }
                    }
                    if(!QRegExp("[A-Za-z][A-Za-z]").exactMatch(fieldTag) && "M5" != fieldTag) { //workaround for bug in the spec
                        throw InvalidFormatException(BAMDbiPlugin::tr("Invalid header field tag: %1").arg(QString(fieldTag)));
                    }
                    if(!QRegExp("[ -~]+").exactMatch(fieldValue)) {
                        throw InvalidFormatException(BAMDbiPlugin::tr("Invalid %1-%2 value: %3").arg(QString(recordTag)).arg(QString(fieldTag)).arg(QString(fieldValue)));
                    }
                    if(!fields.contains(fieldTag)) {
                        fields.insert(fieldTag, fieldValue);
                    } else {
                        throw InvalidFormatException(BAMDbiPlugin::tr("Duplicate header field: %1").arg(QString(fieldTag)));
                    }
                }
            }
            if("HD" == recordTag) {
                if(fields.contains("VN")) {
                    QByteArray value = fields["VN"];
                    if(!QRegExp("[0-9]+\\.[0-9]+").exactMatch(value)) {
                        throw InvalidFormatException(BAMDbiPlugin::tr("Invalid HD-VN value: %1").arg(QString(value)));
                    }
                    header.setFormatVersion(Version::parseVersion(value));
                } else {
                    throw InvalidFormatException(BAMDbiPlugin::tr("HD record without VN field"));
                }
                if(fields.contains("SO")) {
                    QByteArray value = fields["SO"];
                    if("unknown" == value) {
                        header.setSortingOrder(Header::Unknown);
                    } else if("unsorted" == value) {
                        header.setSortingOrder(Header::Unsorted);
                    } else if("queryname" == value) {
                        header.setSortingOrder(Header::QueryName);
                    } else if("coordinate" == value) {
                        header.setSortingOrder(Header::Coordinate);
                    } else if("sorted" == value) { // workaround for invalid headers produced by some programs
                        header.setSortingOrder(Header::Coordinate);
                    } else {
                        throw InvalidFormatException(BAMDbiPlugin::tr("Invalid HD-SO value: %1").arg(QString(value)));
                    }
                }
            } else if("SQ" == recordTag) {
                Header::Reference *reference = NULL;
                if(fields.contains("SN")) {
                    QByteArray value = fields["SN"];
                    if(referencesMap.contains(value)) {
                        reference = &references[referencesMap[value]];
                    } else {
                        throw InvalidFormatException(BAMDbiPlugin::tr("Unmatched SQ-SN value: %1").arg(QString(value)));
                    }
                } else {
                    throw InvalidFormatException(BAMDbiPlugin::tr("SQ record without SN field"));
                }
                if(fields.contains("LN")) {
                    QByteArray value = fields["LN"];
                    bool ok = false;
                    int length = value.toInt(&ok);
                    if(ok) {
                        if(reference->getLength() != length) {
                            throw InvalidFormatException(BAMDbiPlugin::tr("SQ-LN value mismatch: %1 != %2").arg(reference->getLength()).arg(length));
                        }
                    } else {
                        throw InvalidFormatException(BAMDbiPlugin::tr("Invalid SQ-LN value: %1").arg(QString(value)));
                    }
                } else {
                    throw InvalidFormatException(BAMDbiPlugin::tr("SQ record without LN field"));
                }
                if(fields.contains("AS")) {
                    reference->setAssemblyId(fields["AS"]);
                }
                if(fields.contains("M5")) {
                    QByteArray value = fields["M5"];
                    if(!QRegExp("[0-9A-F]+").exactMatch(value)) {
                        throw InvalidFormatException(BAMDbiPlugin::tr("Invalid SQ-M5 value: %1").arg(QString(value)));
                    }
                    reference->setMd5(fields["M5"]);
                }
                if(fields.contains("SP")) {
                    reference->setSpecies(fields["SP"]);
                }
                if(fields.contains("UR")) {
                    reference->setUri(GUrl(fields["UR"]));
                }
            } else if("RG" == recordTag) {
                Header::ReadGroup readGroup;
                if(fields.contains("ID")) {
                    QByteArray value = fields["SN"];
                    readGroupsMap.insert(value, readGroups.size());
                } else {
                    throw InvalidFormatException(BAMDbiPlugin::tr("RG record without ID field"));
                }
                if(fields.contains("CN")) {
                    readGroup.setSequencingCenter(fields["CN"]);
                }
                if(fields.contains("DS")) {
                    readGroup.setDescription(fields["DS"]);
                }
                if(fields.contains("DT")) {
                    QByteArray value = fields["DT"];
                    QDateTime dateTime = QDateTime::fromString(value, Qt::ISODate);
                    if(dateTime.isValid()) {
                        readGroup.setDate(dateTime);
                    } else {
                        QDate date = QDate::fromString(value, Qt::ISODate);
                        if(date.isValid()) {
                            readGroup.setDate(date);
                        } else {
                            throw InvalidFormatException(BAMDbiPlugin::tr("Invalid RG-DT field value: %1").arg(QString(value)));
                        }
                    }
                }
                if(fields.contains("LB")) {
                    readGroup.setLibrary(fields["LB"]);
                }
                if(fields.contains("PG")) {
                    readGroup.setPrograms(fields["PG"]);
                }
                if(fields.contains("PI")) {
                    QByteArray value = fields["PI"];
                    bool ok = false;
                    int predictedInsertSize = value.toInt(&ok);
                    if(ok) {
                        readGroup.setPredictedInsertSize(predictedInsertSize);
                    } else {
                        throw InvalidFormatException(BAMDbiPlugin::tr("Invalid RG-PI field value: %1").arg(QString(value)));
                    }
                }
                if(fields.contains("PL")) {
                    readGroup.setPlatform(fields["PL"]);
                }
                if(fields.contains("PU")) {
                    readGroup.setPlatformUnit(fields["PU"]);
                }
                if(fields.contains("SM")) {
                    readGroup.setSample(fields["SM"]);
                }
                readGroups.append(readGroup);
            } else if("PG" == recordTag) {
                Header::Program program;
                if(fields.contains("ID")) {
                    programsMap.insert(fields["ID"], programs.size());
                } else {
                    throw InvalidFormatException(BAMDbiPlugin::tr("PG record without ID field"));
                }
                if(fields.contains("PN")) {
                    program.setName(fields["PN"]);
                }
                if(fields.contains("CL")) {
                    program.setCommandLine(fields["CL"]);
                }
                if(fields.contains("PP")) {
                    previousProgramIds.append(fields["PP"]);
                } else {
                    previousProgramIds.append(QByteArray());
                }
                if(fields.contains("VN")) {
                    program.setVersion(fields["VN"]);
                }
                programs.append(program);
            }
        }
        for(int index = 0;index < programs.size();index++) {
            const QByteArray &previousProgramId = previousProgramIds[index];
            if(!previousProgramId.isEmpty()) {
                if(programsMap.contains(previousProgramId)) {
                    programs[index].setPreviousId(programsMap[previousProgramId]);
                } else {
                    throw InvalidFormatException(BAMDbiPlugin::tr("Invalid PG-PP field value: %1").arg(QString(previousProgramId)));
                }
            }
        }
        header.setReferences(references);
        header.setReadGroups(readGroups);
        header.setPrograms(programs);
    }
}

} // namespace BAM
} // namespace U2
