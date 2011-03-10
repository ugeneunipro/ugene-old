#ifndef _U2_SARRAY_INDEX_SERIALIZER_H_
#define _U2_SARRAY_INDEX_SERIALIZER_H_

#include "SArrayIndex.h"
#include <U2Core/global.h>
#include <QtCore/QFile>

namespace U2 {

class U2ALGORITHM_EXPORT SArrayIndexSerializer {

public:
    static void serialize(const SArrayIndex *index, const QString &indexFileName, const QString &refFileName);
    static void deserialize(SArrayIndex *index, const QString &indexFileName, TaskStateInfo& ti);

private:
    static const QString SARRAY_HEADER;
    static const QString SARRAY_PARAMETERS;
    static const int BUFF_SIZE = 1048576;
    static void writeArray(QFile &file, char *buff, quint32 *array, int len);
    static void readArray(QFile &file, char *buff, int *len, int *pos, int *bytes,
    int *lineIdx, quint32 *array, int arrLen, TaskStateInfo& ti);
};

} //namespace

#endif
