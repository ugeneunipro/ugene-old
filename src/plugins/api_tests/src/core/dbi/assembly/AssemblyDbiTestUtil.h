#ifndef _ASSEMBLY_DBI_TEST_UTIL_H_
#define _ASSEMBLY_DBI_TEST_UTIL_H_

#include <U2Core/U2Assembly.h>


namespace U2 {

class AssemblyDbiTestUtil {
public:
    static bool compareCigar(const QList<U2CigarToken>& c1, const QList<U2CigarToken>& c2);

    static bool compareReads(const U2AssemblyRead& r1, const U2AssemblyRead& r2);

    static bool findRead(const U2AssemblyRead& subj, QList<U2AssemblyRead>& reads);

    static bool compareReadLists(U2DbiIterator<U2AssemblyRead>* iter, QList<U2AssemblyRead>& expectedReads);

    static void var2readList(const QVariantList& varList, QList<U2AssemblyRead>& reads);

public:
    static const char* ERR_INVALID_ASSEMBLY_ID;
};

} //namespace

Q_DECLARE_METATYPE(U2::U2AssemblyRead);

#endif
