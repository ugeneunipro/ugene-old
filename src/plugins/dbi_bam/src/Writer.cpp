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
#include "Writer.h"

namespace U2 {
namespace BAM {

Writer::Writer(IOAdapter &ioAdapter):
        ioAdapter(ioAdapter),
        writer(ioAdapter)
{
}

namespace {
    void appendField(QByteArray &headerText, const QByteArray &tag, const QByteArray &value) {
        headerText.append("\t");
        headerText.append(tag);
        headerText.append(":");
        headerText.append(value);
    }
}

void Writer::writeHeader(const Header &header) {
    writeBytes(QByteArray("BAM\001"));
    {
        QByteArray headerText;
        {
            headerText.append("@HD");
            appendField(headerText, "VN", header.getFormatVersion().text.toAscii());
            switch(header.getSortingOrder()) {
            case Header::Unknown:
                appendField(headerText, "SO", "unknown");
                break;
            case Header::Unsorted:
                appendField(headerText, "SO", "unsorted");
                break;
            case Header::QueryName:
                appendField(headerText, "SO", "queryname");
                break;
            case Header::Coordinate:
                appendField(headerText, "SO", "coordinate");
                break;
            default:
                assert(false);
            }
            headerText.append("\n");
        }
        foreach(const Header::Reference &reference, header.getReferences()) {
            headerText.append("@SQ");
            appendField(headerText, "SN", reference.getName());
            appendField(headerText, "LN", QByteArray::number(reference.getLength()));
            if(!reference.getAssemblyId().isEmpty()) {
                appendField(headerText, "AS", reference.getAssemblyId());
            }
            if(!reference.getMd5().isEmpty()) {
                appendField(headerText, "M5", reference.getMd5());
            }
            if(!reference.getSpecies().isEmpty()) {
                appendField(headerText, "SP", reference.getSpecies());
            }
            if(!reference.getUri().isEmpty()) {
                appendField(headerText, "UR", reference.getUri().getURLString().toAscii());
            }
            headerText.append("\n");
        }
        for(int index = 0;index < header.getReadGroups().size();index++) {
            const Header::ReadGroup &readGroup = header.getReadGroups()[index];
            headerText.append("@RG");
            appendField(headerText, "ID", QByteArray::number(index));
            if(!readGroup.getSequencingCenter().isEmpty()) {
                appendField(headerText, "CN", readGroup.getSequencingCenter());
            }
            if(!readGroup.getDescription().isEmpty()) {
                appendField(headerText, "DS", readGroup.getDescription());
            }
            switch(readGroup.getDate().type()) {
            case QVariant::Invalid:
                break;
            case QVariant::DateTime:
                appendField(headerText, "DT", readGroup.getDate().toDateTime().toString(Qt::ISODate).toAscii());
                break;
            case QVariant::Date:
                appendField(headerText, "DT", readGroup.getDate().toDate().toString(Qt::ISODate).toAscii());
                break;
            default:
                assert(false);
            }
            if(!readGroup.getLibrary().isEmpty()) {
                appendField(headerText, "LB", readGroup.getLibrary());
            }
            if(!readGroup.getPrograms().isEmpty()) {
                appendField(headerText, "PG", readGroup.getPrograms());
            }
            if(!readGroup.getPlatform().isEmpty()) {
                appendField(headerText, "PL", readGroup.getPlatform());
            }
            if(!readGroup.getPlatformUnit().isEmpty()) {
                appendField(headerText, "PU", readGroup.getPlatformUnit());
            }
            if(!readGroup.getSample().isEmpty()) {
                appendField(headerText, "SM", readGroup.getSample());
            }
            headerText.append("\n");
        }
        for(int index = 0;index < header.getPrograms().size();index++) {
            const Header::Program &program = header.getPrograms()[index];
            headerText.append("@PG");
            appendField(headerText, "ID", QByteArray::number(index));
            if(!program.getName().isEmpty()) {
                appendField(headerText, "PN", program.getName());
            }
            if(!program.getCommandLine().isEmpty()) {
                appendField(headerText, "CL", program.getCommandLine());
            }
            if(-1 != program.getPreviousId()) {
                appendField(headerText, "PP", QByteArray::number(program.getPreviousId()));
            }
            if(!program.getVersion().isEmpty()) {
                appendField(headerText, "VN", program.getVersion());
            }
            headerText.append("\n");
        }
        writeInt32(headerText.size());
        writeBytes(headerText);
    }
    writeInt32(header.getReferences().size());
    foreach(const Header::Reference &reference, header.getReferences()) {
        writeInt32(reference.getName().size() + 1);
        writeString(reference.getName());
        writeInt32(reference.getLength());
    }
}

void Writer::writeRead(const Alignment &alignment) {
    {
        int blockSize = 32 + alignment.getName().size() + 1 + 4*alignment.getCigar().size() + (alignment.getSequence().size() + 1)/2 + alignment.getSequence().size();
        foreach(const QVariant &value, alignment.getOptionalFields()) {
            switch(value.type()) {
            case QVariant::Int:
                {
                    int intValue = value.toInt();
                    if((intValue >= 0) && (intValue <= 0xff)) {
                        blockSize += 1;
                    } else if((intValue >= -0x80) && (intValue <= 0x7f)) {
                        blockSize += 1;
                    } else if((intValue >= 0) && (intValue <= 0xffff)) {
                        blockSize += 2;
                    } else if((intValue >= -0x8000) && (intValue <= 0x7fff)) {
                        blockSize += 2;
                    } else {
                        blockSize += 4;
                    }
                    break;
                }
            case QVariant::UInt:
                blockSize += 4;
                break;
            case QVariant::ByteArray:
                blockSize += value.toByteArray().size() + 1;
                break;
            case QVariant::Char:
                blockSize += 1;
                break;
            case QVariant::Double:
                blockSize += 4;
                break;
            default:
                assert(false);
            }
        }
        writeInt32(blockSize);
    }
    writeInt32(alignment.getReferenceId());
    writeInt32(alignment.getPosition());
    writeUint32((alignment.getBin() << 16) | ((alignment.getMapQuality() << 8) & 0xff) | ((alignment.getName().size() + 1) & 0xff));
    {
        int flagsValue = 0;
        if(alignment.getFlags() & Alignment::Fragmented) {
            flagsValue |= 0x1;
        }
        if(alignment.getFlags() & Alignment::FragmentsAligned) {
            flagsValue |= 0x2;
        }
        if(alignment.getFlags() & Alignment::Unmapped) {
            flagsValue |= 0x4;
        }
        if(alignment.getFlags() & Alignment::NextUnmapped) {
            flagsValue |= 0x8;
        }
        if(alignment.getFlags() & Alignment::Reverse) {
            flagsValue |= 0x10;
        }
        if(alignment.getFlags() & Alignment::NextReverse) {
            flagsValue |= 0x20;
        }
        if(alignment.getFlags() & Alignment::FirstInTemplate) {
            flagsValue |= 0x40;
        }
        if(alignment.getFlags() & Alignment::LastInTemplate) {
            flagsValue |= 0x80;
        }
        if(alignment.getFlags() & Alignment::SecondaryAlignment) {
            flagsValue |= 0x100;
        }
        if(alignment.getFlags() & Alignment::FailsChecks) {
            flagsValue |= 0x200;
        }
        if(alignment.getFlags() & Alignment::Duplicate) {
            flagsValue |= 0x400;
        }
        writeUint32((flagsValue << 16) | (alignment.getCigar().size() & 0xffff));
    }
    writeInt32(alignment.getSequence().size());
    writeInt32(alignment.getNextReferenceId());
    writeInt32(alignment.getNextPosition());
    writeInt32(alignment.getTemplateLength());
    writeString(alignment.getName().mid(0, 0xfe));
    foreach(const Alignment::CigarOperation &operation, alignment.getCigar()) {
        int value = 0;
        switch(operation.getOperation()) {
        case Alignment::CigarOperation::AlignmentMatch:
            value = 0;
            break;
        case Alignment::CigarOperation::Insertion:
            value = 1;
            break;
        case Alignment::CigarOperation::Deletion:
            value = 2;
            break;
        case Alignment::CigarOperation::Skipped:
            value = 3;
            break;
        case Alignment::CigarOperation::SoftClip:
            value = 4;
            break;
        case Alignment::CigarOperation::HardClip:
            value = 5;
            break;
        case Alignment::CigarOperation::Padding:
            value = 6;
            break;
        case Alignment::CigarOperation::SequenceMatch:
            value = 7;
            break;
        case Alignment::CigarOperation::SequenceMismatch:
            value = 8;
            break;
        default:
            assert(false);
        }
        writeUint32((operation.getLength() << 4) | value);
    }
    {
        QByteArray packedSequence((alignment.getSequence().size() + 1)/2, 0);
        for(int index = 0;index < alignment.getSequence().size();index++) {
            int value = 0;
            switch(alignment.getSequence()[index]) {
            case '=':
                value = 0;
                break;
            case 'A':
                value = 1;
                break;
            case 'C':
                value = 2;
                break;
            case 'M':
                value = 3;
                break;
            case 'G':
                value = 4;
                break;
            case 'R':
                value = 5;
                break;
            case 'S':
                value = 6;
                break;
            case 'V':
                value = 7;
                break;
            case 'T':
                value = 8;
                break;
            case 'W':
                value = 9;
                break;
            case 'Y':
                value = 10;
                break;
            case 'H':
                value = 11;
                break;
            case 'K':
                value = 12;
                break;
            case 'D':
                value = 13;
                break;
            case 'B':
                value = 14;
                break;
            case 'X':
            case 'N':
                value = 15;
                break;
            }
            if(0 == (index % 2)) {
                packedSequence[index/2] = packedSequence[index/2] | (value << 4);
            } else {
                packedSequence[index/2] = packedSequence[index/2] | value;
            }
        }
        writeBytes(packedSequence);
    }
    if(alignment.getQuality().isEmpty()) {
        writeBytes(QByteArray(alignment.getSequence().size(), (char)0xff));
    } else {
        assert(alignment.getQuality().size() == alignment.getSequence().size());
        writeBytes(alignment.getQuality());
    }
    foreach(const QByteArray &tag, alignment.getOptionalFields().keys()) {
        assert(2 == tag.size());
        writeBytes(tag);
        QVariant value = alignment.getOptionalFields().value(tag);
        switch(value.type()) {
        case QVariant::Int:
            {
                int intValue = value.toInt();
                if((intValue >= 0) && (intValue <= 0xff)) {
                    writeChar('C');
                    writeUint8(intValue);
                } else if((intValue >= -0x80) && (intValue <= 0x7f)) {
                    writeChar('c');
                    writeInt8(intValue);
                } else if((intValue >= 0) && (intValue <= 0xffff)) {
                    writeChar('S');
                    writeUint16(intValue);
                } else if((intValue >= -0x8000) && (intValue <= 0x7fff)) {
                    writeChar('s');
                    writeInt16(intValue);
                } else {
                    writeChar('i');
                    writeInt32(intValue);
                }
                break;
            }
        case QVariant::UInt:
            writeChar('I');
            writeUint32(value.toUInt());
            break;
        case QVariant::ByteArray:
            writeChar('Z');
            writeString(value.toByteArray());
            break;
        case QVariant::Char:
            writeChar('A');
            writeChar(value.toChar().toLatin1());
            break;
        case QVariant::Double:
            writeChar('f');
            writeFloat32((float)value.toDouble());
            break;
        default:
            assert(false);
        }
    }
}

void Writer::finish() {
    writer.finish();
}

void Writer::writeBytes(const char *buffer, qint64 size) {
    writer.write(buffer, size);
}

void Writer::writeBytes(const QByteArray &buffer) {
    writeBytes(buffer.constData(), buffer.size());
}

void Writer::writeInt32(qint32 value) {
    char buffer[4];
    buffer[0] = (char)(value >> 0);
    buffer[1] = (char)(value >> 8);
    buffer[2] = (char)(value >> 16);
    buffer[3] = (char)(value >> 24);
    writeBytes(buffer, sizeof(buffer));
}

void Writer::writeUint32(quint32 value) {
    char buffer[4];
    buffer[0] = (char)(value >> 0);
    buffer[1] = (char)(value >> 8);
    buffer[2] = (char)(value >> 16);
    buffer[3] = (char)(value >> 24);
    writeBytes(buffer, sizeof(buffer));
}

void Writer::writeInt16(qint16 value) {
    char buffer[2];
    buffer[0] = (char)(value >> 0);
    buffer[1] = (char)(value >> 8);
    writeBytes(buffer, sizeof(buffer));
}

void Writer::writeUint16(quint16 value) {
    char buffer[2];
    buffer[0] = (char)(value >> 0);
    buffer[1] = (char)(value >> 8);
    writeBytes(buffer, sizeof(buffer));
}

void Writer::writeInt8(qint8 value) {
    char buffer[1];
    buffer[0] = (char)(value >> 0);
    writeBytes(buffer, sizeof(buffer));
}

void Writer::writeUint8(quint8 value) {
    char buffer[1];
    buffer[0] = (char)(value >> 0);
    writeBytes(buffer, sizeof(buffer));
}

void Writer::writeFloat32(float value) {
    quint32 *pointer = (quint32 *)&value;
    writeUint32(*pointer);
}

void Writer::writeChar(char value) {
    writeBytes(&value, 1);
}

void Writer::writeString(const QByteArray &string) {
    writeBytes(string);
    writeBytes(QByteArray(1, '\0'));
}

} // namespace BAM
} // namespace U2
