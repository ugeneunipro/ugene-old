#include "FileDbi.h"

#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/L10n.h>

namespace U2 {

MsaObjectDbi::MsaObjectDbi( FileDbi* rootDbi ) : U2MsaRWDbi(rootDbi), root(rootDbi)
{
}

void MsaObjectDbi::createMsaObject( U2Msa& msa, const QString& folder, U2OpStatus& os )
{
    QWriteLocker lock(root->sync);
    if (!root->canCreate(GObjectTypes::MULTIPLE_ALIGNMENT, os)) {
        return;
    }
    if (!root->checkFolder(folder, os)) {
        return;
    }
    DNAAlphabet* al = root->checkAlphabet(msa.alphabet.id, os);
    if (al == NULL) {
        return;
    }

    QString name = root->newName(GObjectTypes::MULTIPLE_ALIGNMENT);
    MAlignment ma(name, al);
    MAlignmentObject* mao = new MAlignmentObject(ma);
    root->doc->addObject(mao);
    msa.dbiId = root->getDbiId();
    msa.id = root->obj2id(mao);

    lock.unlock();
    root->folderDbi->addObjectsToFolder(QList<U2DataId>() << msa.id, folder, os);
}

void MsaObjectDbi::removeSequences( U2Msa& msa, const QList<U2DataId> sequenceIds, U2OpStatus& os)
{
    QWriteLocker lock(root->sync);
    if (!root->canModify(os)) {
        return;
    }
    MAlignmentObject* mao = root->findMA(msa.id, os);
    QList<int> rowIds;
    QList<DNASequenceObject*> gos = root->msaSeqs.values(mao);
    QList<U2DataId> sids = sequenceIds;
    for(int i = 0; i < gos.length(); i++) {
        U2DataId id = root->obj2id(gos[i]);
        if (sids.removeAll(id) != 0) {
            rowIds << i;
        }
    }
    if (!sids.isEmpty()) {
        os.setError(L10N::badArgument("no such rows"));
    }
    qStableSort(rowIds.begin(), rowIds.end(), qGreater<int>());
    foreach(int id, rowIds) {
        mao->removeRow(id);
    }

    lock.unlock();
}

void MsaObjectDbi::addSequences( U2Msa& msa, const QList<U2MsaRow>& rows, U2OpStatus& os)
{
    QWriteLocker lock(root->sync);
    if (!root->canModify(os)) {
        return;
    }
    MAlignmentObject* mao = root->findMA(msa.id, os);
    QList<DNASequenceObject*> gos;
    foreach(const U2MsaRow& r, rows) {
        DNASequenceObject* go = root->findSeq(r.sequenceId, os);
        if (go == NULL) {
            return;
        }
        if (root->msaSeqs.contains(mao, go)) {
            os.setError("Sequence is already added to MSA");
            return;
        }
        assert(!gos.contains(go));
        gos << go;
    }
    int scount = mao->getMAlignment().getNumRows();
    for (int i = 0; i < gos.length(); i++) {
        root->msaSeqs.insert(mao, gos[i]);
        mao->addRow(gos.at(i)->getDNASequence());
        foreach(const U2MsaGap& gap, rows.at(i).gaps) {
            mao->insertGap(scount, gap.offset, gap.gap);
        }
        scount++;
    }
    msa.alphabet = mao->getMAlignment().getAlphabet()->getId();

    lock.unlock();
}

U2::U2Msa MsaObjectDbi::getMsaObject(U2DataId id, U2OpStatus& os ) {
    U2Msa msa;
    QReadLocker lock(root->sync);
    MAlignmentObject* mao = root->findMA(id, os);
    if (mao == NULL) {
        return msa;
    }

    msa.id = id;
    msa.dbiId = root->getDbiId();
    msa.alphabet.id = mao->getAlphabet()->getId();
    return msa;
}

static QList<U2MsaGap> seq2gap(const QByteArray& data) {
    QList<U2MsaGap> gaps;
    for (int i = 0; i < data.size(); i++) {
        if (data[i] == MAlignment_GapChar) {
            if (!gaps.isEmpty() && gaps.last().offset == i - 1) {
                gaps.last().gap++;
            } else {
                gaps << U2MsaGap(i, 1);
            }
        }
    }
    return gaps;
}

QList<U2MsaRow> MsaObjectDbi::getRows(U2DataId id, qint32 fromRow, qint32 numRows, U2OpStatus& os) 
{
    QList<U2MsaRow> res;
    QReadLocker lock(root->sync);
    MAlignmentObject* mao = root->findMA(id, os);
    if (mao == NULL) {
        return res;
    }
    
    QList<DNASequenceObject*> rows = root->msaSeqs.values(mao);
    if (fromRow < 0 || numRows < 0 || fromRow + numRows > rows.size()) {
        os.setError(L10N::badArgument("rows region"));
        return res;
    }
    for (int i = fromRow ; i < fromRow + numRows; i++) {
        DNASequenceObject* rowSeq = rows.at(i);
        U2MsaRow row;
        row.sequenceId = root->obj2id(rowSeq);
        row.gaps = seq2gap(mao->getMAlignment().getRow(i).toByteArray(mao->getLength()));
        res << row;
    }
    return res;
}


qint32 MsaObjectDbi::countSequencesAt( U2DataId id, qint64 coord, U2OpStatus& os ) 
{
    qint32 res = 0;
    QReadLocker lock(root->sync);
    MAlignmentObject* mao = root->findMA(id, os);
    if (mao == NULL) {
        return res;
    }
    if (coord < 0 || mao->getLength() <= coord) {
        os.setError(L10N::badArgument("coord"));
        return res;
    }

    foreach(const MAlignmentRow& r, mao->getMAlignment().getRows()) {
        if (r.getFirstNonGapIdx() <= coord && coord <= r.getLastNonGapIdx()) {
            res++;
        }
    }
    return res;
}

qint32 MsaObjectDbi::countSequencesAt( U2DataId id, const U2Region& r, U2OpStatus& os ) 
{
    qint64 res = 0;
    QReadLocker lock(root->sync);
    MAlignmentObject* mao = root->findMA(id, os);
    if (mao == NULL) {
        return res;
    }
    if (r.startPos < 0 || mao->getLength() < r.endPos()) {
        os.setError(L10N::badArgument("region"));
        return res;
    }

    foreach(const MAlignmentRow& row, mao->getMAlignment().getRows()) {
        U2Region rowRegion(row.getFirstNonGapIdx(), row.getLastNonGapIdx()-row.getFirstNonGapIdx());
        if (r.intersects(rowRegion)) {
            res++;
        }
    }
    return res;
}

QList<U2DataId> MsaObjectDbi::getSequencesAt( U2DataId id, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os ) {
    QList<U2DataId> res;
    QReadLocker lock(root->sync);
    MAlignmentObject* mao = root->findMA(id, os);
    if (mao == NULL) {
        return res;
    }
    if (coord < 0 || mao->getLength() <= coord) {
        os.setError(L10N::badArgument("coord"));
        return res;
    }
    if (offset < 0) {
        os.setError(L10N::badArgument("offset"));
        return res;
    }

    if (count < -1) {
        os.setError(L10N::badArgument("count"));
        return res;
    }

    QList<DNASequenceObject*> rows = root->msaSeqs.values(mao);
    for(int i = 0; i < mao->getMAlignment().getNumRows(); i++) {
        const MAlignmentRow& r = mao->getMAlignment().getRow(i);
        //first non gap character is <= coord <= last non gap character
        if (r.getFirstNonGapIdx() <= coord && coord <= r.getLastNonGapIdx()) {
            DNASequenceObject* rowSeq = rows.at(i);
            res << root->obj2id(rowSeq);
            if (count != U2_DBI_NO_LIMIT && res.size() == offset + count) {
                break;
            }
        }
    }
    return res.mid(offset, count);
}

QList<U2DataId> MsaObjectDbi::getSequencesAt( U2DataId id, const U2Region& r, qint32 offset, qint32 count, U2OpStatus& os ) 
{
    QList<U2DataId> res;
    QReadLocker lock(root->sync);
    MAlignmentObject* mao = root->findMA(id, os);
    if (mao == NULL) {
        return res;
    }
    if (r.startPos < 0 || mao->getLength() < r.endPos()) {
        os.setError(L10N::badArgument("region"));
        return res;
    }
    if (offset < 0) {
        os.setError(L10N::badArgument("offset"));
        return res;
    }

    if (count < -1) {
        os.setError(L10N::badArgument("count"));
        return res;
    }

    QList<DNASequenceObject*> rows = root->msaSeqs.values(mao);
    for(int i = 0; i < mao->getMAlignment().getNumRows(); i++) {
        const MAlignmentRow& row = mao->getMAlignment().getRow(i);
        if (r.intersects(U2Region(row.getFirstNonGapIdx(), row.getLastNonGapIdx()))) {
            DNASequenceObject* rowSeq = rows.at(i);
            res << root->obj2id(rowSeq);
            if (count != U2_DBI_NO_LIMIT && res.size() == offset + count) {
                break;
            }
        }
    }
    return res.mid(offset, count);
}

qint32 MsaObjectDbi::countSequencesWithoutGapAt( U2DataId id, qint64 coord, U2OpStatus& os ) {
    qint64 res = 0;
    QReadLocker lock(root->sync);
    MAlignmentObject* mao = root->findMA(id, os);
    if (mao == NULL) {
        return res;
    }
    if (coord < 0 || mao->getLength() <= coord) {
        os.setError(L10N::badArgument("coord"));
        return res;
    }

    foreach(const MAlignmentRow& r, mao->getMAlignment().getRows()) {
        if (r.chatAt(coord) != MAlignment_GapChar) {
            res++;
        }
    }
    return res;
}

QList<U2DataId> MsaObjectDbi::getSequencesWithoutGapAt( U2DataId id, qint64 coord, qint32 offset, qint32 count, U2OpStatus& os ) {
    QList<U2DataId> res;
    QReadLocker lock(root->sync);
    MAlignmentObject* mao = root->findMA(id, os);
    if (mao == NULL) {
        return res;
    }
    if (coord < 0 || mao->getLength() <= coord) {
        os.setError(L10N::badArgument("coord"));
        return res;
    }
    if (offset < 0) {
        os.setError(L10N::badArgument("offset"));
        return res;
    }

    if (count < -1) {
        os.setError(L10N::badArgument("count"));
        return res;
    }

    QList<DNASequenceObject*> rows = root->msaSeqs.values(mao);
    for(int i = 0; i < mao->getMAlignment().getNumRows(); i++) {
        const MAlignmentRow& r = mao->getMAlignment().getRow(i);
        if (r.chatAt(coord) != MAlignment_GapChar) {
            DNASequenceObject* rowSeq = rows.at(i);
            res << root->obj2id(rowSeq);
            if (count != U2_DBI_NO_LIMIT && res.size() == offset + count) {
                break;
            }
        }
    }
    return res.mid(offset, count);
}

qint64 MsaObjectDbi::getSequencesCount(U2DataId msaId, U2OpStatus& os) {
    QReadLocker lock(root->sync);
    MAlignmentObject* mao = root->findMA(msaId, os);
    if (mao == NULL) {
        return -1;
    }
    return mao->getMAlignment().getNumRows();
}


AssemblyObjectDbi::AssemblyObjectDbi( FileDbi* rootDbi ): U2AssemblyRWDbi(rootDbi), root(rootDbi)
{
}

U2Assembly AssemblyObjectDbi::getAssemblyObject( U2DataId id, U2OpStatus& os ) {
    U2Assembly msa;
    QReadLocker lock(root->sync);
    MAlignmentObject* mao = root->findMA(id, os);
    if (mao == NULL) {
        return msa;
    }

    msa.id = id;
    msa.dbiId = root->getDbiId();
    msa.visualName = mao->getGObjectName();
    //TODO ref
    return msa;
}


qint64 AssemblyObjectDbi::countReadsAt( U2DataId msaId, const U2Region& r, U2OpStatus& os ) {
    return root->msaDbi->countSequencesAt(msaId, r, os);
}

QList<U2DataId> AssemblyObjectDbi::getReadIdsAt( U2DataId msaId, const U2Region& r, qint64 offset, qint64 count, U2OpStatus& os ) {
    return root->msaDbi->getSequencesAt(msaId, r, offset, count, os);
}

QList<U2AssemblyRead> AssemblyObjectDbi::getReadsAt(U2DataId assemblyId, const U2Region& r, qint64 offset, qint64 count, U2OpStatus& os) {
    QList<U2AssemblyRead> res;
    os.setError("Unsupported operation"); //TODO
    return res;
}

qint64 AssemblyObjectDbi::getMaxPackedRow(U2DataId assemblyId, const U2Region& r, U2OpStatus& os) {
    os.setError("Unsupported operation"); //TODO
    return 0;
}

QList<U2AssemblyRead> AssemblyObjectDbi::getReadsByRow(U2DataId assemblyId, const U2Region& r, qint64 minRow, qint64 maxRow, U2OpStatus& os) {
    QList<U2AssemblyRead> res;
    os.setError("Unsupported operation"); //TODO
    return res;
}

void AssemblyObjectDbi::createAssemblyObject( U2Assembly& msa, const QString& folder, U2AssemblyReadsIterator* it, U2OpStatus& os )
{
    os.setError("Unsupported operation"); //TODO
}

void AssemblyObjectDbi::removeReads( U2DataId assemblyId, const QList<U2DataId>& rowIds, U2OpStatus& os)
{
    os.setError("Unsupported operation"); //TODO
}

void AssemblyObjectDbi::addReads( U2DataId assemblyId, QList<U2AssemblyRead>& rows, U2OpStatus& os)
{
    os.setError("Unsupported operation"); //TODO
}

void AssemblyObjectDbi::pack( U2DataId assemblyId, U2OpStatus& os)
{
    os.setError("Unsupported operation"); //TODO
}

/** Count 'length of assembly' - position of the rightmost base of all reads */
quint64 AssemblyObjectDbi::getMaxEndPos(U2DataId assemblyId, U2OpStatus& os) {
    //todo:
    return -1;
}

/** Return assembly row structure by id */
U2AssemblyRead AssemblyObjectDbi::getReadById(U2DataId rowId, U2OpStatus& os) {
    U2AssemblyRead res;
    os.setError("Unsupported operation"); //TODO
    return res;
}

}//ns