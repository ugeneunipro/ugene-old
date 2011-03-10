#include "BAMDbiPlugin.h"
#include "InvalidFormatException.h"
#include "IOException.h"
#include "BaiReader.h"

namespace U2 {
namespace BAM {

BaiReader::BaiReader(IOAdapter &ioAdapter):
    ioAdapter(ioAdapter)
{
}

Index BaiReader::readIndex() {
    {
        QByteArray magic = readBytes(4);
        if("BAI\001" != magic) {
            throw InvalidFormatException(BAMDbiPlugin::tr("invalid magic number"));
        }
    }
    int referencesNumber = readInt32();
    if(referencesNumber < 0) {
        throw InvalidFormatException(BAMDbiPlugin::tr("invalid number of references: %1").arg(referencesNumber));
    }
    QList<Index::ReferenceIndex> referenceIndices;
    for(int referenceId = 0;referenceId < referencesNumber;referenceId++) {
        int binsNumber = readInt32();
        if(binsNumber < 0) {
            throw InvalidFormatException(BAMDbiPlugin::tr("invalid number of bins: %1").arg(binsNumber));
        }
        QList<QList<Index::ReferenceIndex::Chunk> > bins;
        for(int i = 0;i < 37500;i++) {
            bins.append(QList<Index::ReferenceIndex::Chunk>());
        }
        for(int binId = 0;binId < binsNumber;binId++) {
            unsigned int bin = readUint32();
            int chunksNumber = readInt32();
            if(chunksNumber < 0) {
                throw InvalidFormatException(BAMDbiPlugin::tr("invalid number of chunks: %1").arg(chunksNumber));
            }
            for(int chunkId = 0;chunkId < chunksNumber;chunkId++) {
                VirtualOffset chunkBegin(readUint64());
                VirtualOffset chunkEnd(readUint64());
                if(chunkEnd < chunkBegin) {
                    throw InvalidFormatException(BAMDbiPlugin::tr("invalid chunk"));
                }
                bins[bin].append(Index::ReferenceIndex::Chunk(chunkBegin, chunkEnd));
            }
        }
        int intervalsNumber = readInt32();
        if(intervalsNumber < 0) {
            throw InvalidFormatException(BAMDbiPlugin::tr("invalid number of intervals: %1").arg(intervalsNumber));
        }
        QList<VirtualOffset> intervals;
        for(int intervalId = 0;intervalId < intervalsNumber;intervalId++) {
            VirtualOffset intervalOffset(readUint64());
            intervals.append(intervalOffset);
        }
        referenceIndices.append(Index::ReferenceIndex(bins, intervals));
    }
    return Index(QList<Index::ReferenceIndex>(referenceIndices));
}

void BaiReader::readBytes(char *buff, qint64 size) {
    qint64 returnedValue = ioAdapter.readBlock(buff, size);
    if(-1 == returnedValue) {
        throw IOException(BAMDbiPlugin::tr("can't read input"));
    } else if(returnedValue < size) {
        throw InvalidFormatException(BAMDbiPlugin::tr("unexpected end of file"));
    }
}

QByteArray BaiReader::readBytes(qint64 size) {
    QByteArray result(size, 0);
    readBytes(result.data(), result.size());
    return result;
}

quint64 BaiReader::readUint64() {
    char buffer[8];
    readBytes(buffer, sizeof(buffer));
    return (quint64)(buffer[0] & 0xff) |
            ((quint64)(buffer[1] & 0xff) << 8) |
            ((quint64)(buffer[2] & 0xff) << 16) |
            ((quint64)(buffer[3] & 0xff) << 24) |
            ((quint64)(buffer[4] & 0xff) << 32) |
            ((quint64)(buffer[5] & 0xff) << 40) |
            ((quint64)(buffer[6] & 0xff) << 48) |
            ((quint64)(buffer[7] & 0xff) << 56);
}

qint32 BaiReader::readInt32() {
    char buffer[4];
    readBytes(buffer, sizeof(buffer));
    return (buffer[0] & 0xff) |
            ((buffer[1] & 0xff) << 8) |
            ((buffer[2] & 0xff) << 16) |
            ((buffer[3] & 0xff) << 24);
}

quint64 BaiReader::readUint32() {
    char buffer[4];
    readBytes(buffer, sizeof(buffer));
    return (buffer[0] & 0xff) |
            ((buffer[1] & 0xff) << 8) |
            ((buffer[2] & 0xff) << 16) |
            ((buffer[3] & 0xff) << 24);
}

} // namespace BAM
} // namespace U2
