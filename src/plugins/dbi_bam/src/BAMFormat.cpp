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

//using 3rd-party zlib (not included in ugene bundle) on *nix
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
#include <zlib.h>
#else
#include "zlib.h"
#endif

#include <U2Formats/DocumentFormatUtils.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/Task.h>
#include <U2Core/Version.h>
#include "Reader.h"
#include "Writer.h"
#include "BAMDbiPlugin.h"
#include "Exception.h"
#include "BAMFormat.h"

namespace U2 {
namespace BAM {

const QString BAMFormat::FORMAT_ID = "bam";

BAMFormat::BAMFormat(QObject *parent):
    DocumentFormat(parent, DocumentFormatFlag_SupportWriting, QStringList("bam")),
    formatName(tr("BAM"))
{
    supportedObjectTypes += GObjectTypes::MULTIPLE_ALIGNMENT;
}

DocumentFormatId BAMFormat::getFormatId()const {
    return FORMAT_ID;
}

const QString &BAMFormat::getFormatName()const {
    return formatName;
}

Document *BAMFormat::loadDocument(IOAdapter *io, TaskStateInfo &ti, const QVariantMap &fs, DocumentLoadMode mode) {
    assert(DocumentLoadMode_Whole == mode);
    Q_UNUSED(fs);
    Q_UNUSED(mode);
    try {
        QList<GObject *> objects;
        QVariantMap hints;
        QString lockReason;
        QList<MAlignment> alignments;
        {
            BamReader reader(*io);
            foreach(const Header::Reference &reference, reader.getHeader().getReferences()) {
                MAlignment alignment(reference.getName());
                alignment.setLength(reference.getLength());
                alignments.push_back(alignment);
            }
            while(!reader.isEof() && !ti.cancelFlag) {
                Alignment alignment = reader.readAlignment();
                if(!(alignment.getFlags() & Unmapped) &&
                   !alignment.getSequence().isEmpty() &&
                   !alignment.getCigar().isEmpty() &&
                   (-1 != alignment.getPosition()) &&
                   (-1 != alignment.getReferenceId())) {
                    MAlignmentRow row(alignment.getName());
                    {
                        QByteArray sequence;
                        QByteArray quality;
                        int position = 0;
                        foreach(const Alignment::CigarOperation &operation, alignment.getCigar()) {
                            switch(operation.getOperation()) {
                            case Alignment::CigarOperation::AlignmentMatch:
                            case Alignment::CigarOperation::SequenceMatch:
                            case Alignment::CigarOperation::SequenceMismatch:
                            case Alignment::CigarOperation::SoftClip:
                                sequence.append(alignment.getSequence().mid(position, operation.getLength()));
                                if(!alignment.getQuality().isEmpty()) {
                                    quality.append(alignment.getQuality().mid(position, operation.getLength()));
                                }
                                position += operation.getLength();
                                break;
                            case Alignment::CigarOperation::Insertion:
                                position += operation.getLength();
                                break;
                            default:
                                sequence.append(QByteArray(operation.getLength(), MAlignment_GapChar));
                                if(!alignment.getQuality().isEmpty()) {
                                    quality.append(QByteArray(operation.getLength(), DNAQuality::encode(
                                            0, DNAQualityType_Sanger)));
                                }
                            }
                        }
                        row.setSequence(sequence, alignment.getPosition());
                        if(!alignment.getQuality().isEmpty()) {
                            row.setQuality(DNAQuality(quality));
                        }
                    }
                    alignments[alignment.getReferenceId()].addRow(row);
                }
                ti.progress = io->getProgress();
            }
        }
        foreach(MAlignment alignment, alignments) {
            DocumentFormatUtils::assignAlphabet(alignment);
            if(NULL == alignment.getAlphabet()) {
                throw Exception(BAMDbiPlugin::tr("Alphabet is unknown"));
            }
            objects.push_back(new MAlignmentObject(alignment));
        }
        if(ti.cancelFlag) {
            return NULL;
        }
        return new Document(this, io->getFactory(), io->getURL(), objects, hints, lockReason);
    } catch(const Exception &e) {
        ti.setError(e.getMessage());
        return NULL;
    }
}

void BAMFormat::storeDocument(Document *d, TaskStateInfo &ts, IOAdapter *io) {
    try {
        Writer writer(*io);
        QList<MAlignmentObject *> alignments;
        foreach(GObject *object, d->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT)) {
            alignments.push_back(dynamic_cast<MAlignmentObject *>(object));
        }
        {
            Header header;
            {
                QList<Header::Reference> references;
                foreach(MAlignmentObject *object, alignments) {
                    const MAlignment &alignment = object->getMAlignment();
                    references.push_back(Header::Reference(alignment.getName().toAscii(), alignment.getLength()));
                }
                header.setReferences(references);
            }
            header.setFormatVersion(Version::parseVersion("1.0"));
            header.setSortingOrder(Header::Unsorted); // TODO: sort rows
            writer.writeHeader(header);
        }
        {
            int referenceId = 0;
            foreach(MAlignmentObject *object, alignments) {
                const MAlignment &alignment = object->getMAlignment();
                // TODO: sort rows
                foreach(const MAlignmentRow &row, alignment.getRows()) {
                    Alignment alignment;
                    alignment.setReferenceId(referenceId);
                    alignment.setName(row.getName().toAscii());
                    alignment.setPosition(row.getFirstNonGapIdx());
                    {
                        QByteArray sequence;
                        QByteArray quality;
                        QList<Alignment::CigarOperation> cigar;
                        int start = row.getFirstNonGapIdx();
                        int end = row.getLastNonGapIdx() + 1;
                        bool gap = true;
                        for(int index = start;index < end;index++) {
                            if(MAlignment_GapChar == row.chatAt(index)) {
                                if(!gap) {
                                    cigar.push_back(Alignment::CigarOperation(0, Alignment::CigarOperation::Deletion));
                                    gap = true;
                                }
                            } else {
                                sequence.push_back(row.chatAt(index));
                                if(row.hasQuality()) {
                                    quality.push_back(DNAQuality::encode(
                                            row.getCoreQuality().getValue(index - row.getCoreStart()),
                                            DNAQualityType_Sanger));
                                } else {
                                    quality.push_back((char)0xff);
                                }
                                if(gap) {
                                    cigar.push_back(Alignment::CigarOperation(0, Alignment::CigarOperation::AlignmentMatch));
                                    gap = false;
                                }
                            }
                            cigar.back().setLength(cigar.back().getLength() + 1);
                        }
                        alignment.setSequence(sequence);
                        alignment.setQuality(quality);
                        alignment.setCigar(cigar);
                    }
                    writer.writeRead(alignment);
                    if(ts.cancelFlag) {
                        break;
                    }
                }
                referenceId++;
                if(ts.cancelFlag) {
                    break;
                }
            }
        }
        // TODO: build index
        writer.finish();
    } catch(const Exception &e) {
        ts.setError(e.getMessage());
    }
}

FormatDetectionResult BAMFormat::checkRawData(const QByteArray &rawData, const GUrl & /*url*/) const {
    z_stream_s stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.next_in = (Bytef *)rawData.constData();
    stream.avail_in = rawData.size();
    QByteArray magic(4, '\0');
    stream.next_out = (Bytef *)magic.data();
    stream.avail_out = magic.size();
    FormatDetectionResult result = FormatDetection_NotMatched;
    if(Z_OK == inflateInit2(&stream, 16 + 15)) {
        if(Z_OK == inflate(&stream, Z_SYNC_FLUSH)) {
            if(0 == stream.avail_out) {
                if("BAM\001" == magic) {
                    result = FormatDetection_Matched;
                }
            }
        }
        inflateEnd(&stream);
    }
    return result;
}

} // namespace BAM
} // namespace U2
