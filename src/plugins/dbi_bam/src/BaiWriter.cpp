#include "BAMDbiPlugin.h"
#include "IOException.h"
#include "BaiWriter.h"

namespace U2 {
namespace BAM {

BaiWriter::BaiWriter(IOAdapter &ioAdapter):
    ioAdapter(ioAdapter)
{
}

void BaiWriter::writeIndex(const Index &index) {
    writeBytes(QByteArray("BAI\001"));
    writeInt32(index.getReferenceIndices().size());
    foreach(const Index::ReferenceIndex &referenceIndex, index.getReferenceIndices()) {
        int binsNumber = 0;
        foreach(const QList<Index::ReferenceIndex::Chunk> &bin, referenceIndex.getBins()) {
            if(!bin.isEmpty()) {
                binsNumber++;
            }
        }
        writeInt32(referenceIndex.getBins().size());
        for(int binId = 0;binId < referenceIndex.getBins().size();binId++) {
            const QList<Index::ReferenceIndex::Chunk> &bin = referenceIndex.getBins()[binId];
            if(!bin.isEmpty()) {
                writeUint32(binId);
                writeInt32(bin.size());
                foreach(const Index::ReferenceIndex::Chunk &chunk, bin) {
                    writeUint64(chunk.getStart().getPackedOffset());
                    writeUint64(chunk.getEnd().getPackedOffset());
                }
            }
        }
        writeInt32(referenceIndex.getIntervals().size());
        foreach(const VirtualOffset &offset, referenceIndex.getIntervals()) {
            writeUint64(offset.getPackedOffset());
        }
    }
}

void BaiWriter::writeBytes(const char *buff, qint64 size) {
    if(ioAdapter.writeBlock(buff, size) != size) {
        throw IOException(BAMDbiPlugin::tr("can't write output"));
    }
}

void BaiWriter::writeBytes(const QByteArray &buffer) {
    writeBytes(buffer.constData(), buffer.size());
}

void BaiWriter::writeUint64(quint64 value) {
    char buffer[8];
    buffer[0] = (char)(value >> 0);
    buffer[1] = (char)(value >> 8);
    buffer[2] = (char)(value >> 16);
    buffer[3] = (char)(value >> 24);
    buffer[4] = (char)(value >> 32);
    buffer[5] = (char)(value >> 40);
    buffer[6] = (char)(value >> 48);
    buffer[7] = (char)(value >> 56);
    writeBytes(buffer, sizeof(buffer));
}

void BaiWriter::writeInt32(qint32 value) {
    char buffer[4];
    buffer[0] = (char)(value >> 0);
    buffer[1] = (char)(value >> 8);
    buffer[2] = (char)(value >> 16);
    buffer[3] = (char)(value >> 24);
    writeBytes(buffer, sizeof(buffer));
}

void BaiWriter::writeUint32(quint32 value) {
    char buffer[4];
    buffer[0] = (char)(value >> 0);
    buffer[1] = (char)(value >> 8);
    buffer[2] = (char)(value >> 16);
    buffer[3] = (char)(value >> 24);
    writeBytes(buffer, sizeof(buffer));
}

} // namespace BAM
} // namespace U2
