#ifndef _GB2_UHMM3_SEARCH_H_
#define _GB2_UHMM3_SEARCH_H_

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

#include <U2Core/Task.h>

#include <hmmer3/hmmer.h>

#include "uhmm3SearchResult.h"

namespace U2 {

class UHMM3Search : public QObject {
    Q_OBJECT
public:
    static UHMM3SearchResult search( const P7_HMM* ahmm, const char* sq, int sqLen, 
                                     const UHMM3SearchSettings& set, TaskStateInfo& si, int wholeSeqSz );
    
}; // UHMM3Search

} // U2

#endif // _GB2_UHMM3_SEARCH_H_
