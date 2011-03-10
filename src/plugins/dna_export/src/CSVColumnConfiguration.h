#ifndef _U2_CSV_COLUMN_CONFIGURATION_H_
#define _U2_CSV_COLUMN_CONFIGURATION_H_

#include <QtCore/QString>

namespace U2 {

enum ColumnRole {
    ColumnRole_Ignore,      // ignore this column
    ColumnRole_Qualifier,   // column will be mapped to a qualifier
    ColumnRole_Name,        // name (or key) of the annotation
    ColumnRole_StartPos,    // column will be mapped as start position
    ColumnRole_EndPos,      // column will be mapped as end position
    ColumnRole_Length,      // column will be mapped as length
    ColumnRole_ComplMark    // column is a complement strand indicator
};

class ColumnConfig {
public:
    ColumnConfig() : role (ColumnRole_Ignore), startPositionOffset(0), endPositionIsInclusive(false) {}
    ColumnRole  role;
    QString     qualifierName;
    QString     complementMark;
    int         startPositionOffset;
    bool        endPositionIsInclusive;

    void reset() {
        role = ColumnRole_Ignore;
        qualifierName.clear();
        complementMark.clear();
        startPositionOffset = 0;
        endPositionIsInclusive = false;
    }

    //TODO: support groups?
};

}//namespace

#endif
