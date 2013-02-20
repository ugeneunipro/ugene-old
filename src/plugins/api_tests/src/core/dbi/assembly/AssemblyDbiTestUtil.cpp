#include <SamtoolsAdapter.h>
#include "AssemblyDbiTestUtil.h"


namespace U2 {

const char* AssemblyDbiTestUtil::ERR_INVALID_ASSEMBLY_ID("Error message for invalid assembly id passed is not set.");

bool AssemblyDbiTestUtil::compareCigar( const QList<U2CigarToken>& c1, const QList<U2CigarToken>& c2 ) {
    if (c1.size() != c2.size()) {
        return false;
    }
    for (int i=0; i<c1.size(); i++) {
        U2CigarToken tok1 = c1.at(i);
        U2CigarToken tok2 = c2.at(i);
        if (tok1.count != tok2.count || tok1.op != tok2.op) {
            return false;
        }
    }
    return true;
}

bool AssemblyDbiTestUtil::compareReads( const U2AssemblyRead& r1, const U2AssemblyRead& r2 ) {
    if (r1->name != r2->name) {
        return false;
    }
    if (r1->leftmostPos != r2->leftmostPos) {
        return false;
    }
    if (r1->effectiveLen != r2->effectiveLen) {
        return false;
    }
    if (r1->packedViewRow != r2->packedViewRow) {
        return false;
    }
    if (r1->readSequence != r2->readSequence) {
        return false;
    }
    bool q1 = SamtoolsAdapter::hasQuality(r1->quality);
    bool q2 = SamtoolsAdapter::hasQuality(r2->quality);
    if (q1 != q2) {
        return false;
    }
    if (q1 && (r1->quality != r2->quality)) {
        return false;
    }
    if (r1->mappingQuality != r2->mappingQuality) {
        return false;
    }
    if (r1->flags != r2->flags) {
        return false;
    }
    if (!compareCigar(r1->cigar, r2->cigar)) {
        return false;
    }
    return true;
}

bool AssemblyDbiTestUtil::findRead( const U2AssemblyRead& subj, QList<U2AssemblyRead>& reads ) {
    for (qint64 i=0, n = reads.size(); i<n; i++) {
        const U2AssemblyRead& curRead = reads.at(i);
        if (compareReads(subj, curRead)) {
            reads.removeAt(i);
            return true;
        }
    }
    return false;
}

bool AssemblyDbiTestUtil::compareReadLists( U2DbiIterator<U2AssemblyRead>* iter, QList<U2AssemblyRead>& expectedReads ) {
    while (iter->hasNext()) {
        const U2AssemblyRead& read = iter->next();
        if ( !findRead(read, expectedReads) ) {
            return false;
        }
    }
    if (!expectedReads.isEmpty()) {
        return false;
    }
    return true;
}

void AssemblyDbiTestUtil::var2readList( const QVariantList& varList, QList<U2AssemblyRead>& reads ) {
    foreach(QVariant var, varList) {
        U2AssemblyRead read = qVariantValue<U2AssemblyRead>(var);
        reads.append(read);
    }
}

} //namespace
