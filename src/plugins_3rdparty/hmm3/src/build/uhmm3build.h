#ifndef _GB2_UHMM3_BUILD_H_
#define  _GB2_UHMM3_BUILD_H_

#include <QtCore/QObject>

#include <U2Core/MAlignment.h>
#include <U2Core/Task.h>

#include <hmmer3/hmmer.h>

namespace U2 {

class UHMM3Build : public QObject {
    Q_OBJECT
public:
    static P7_HMM * build( const MAlignment & msa, const UHMM3BuildSettings & settings ,TaskStateInfo & ti );
    
}; // UHMM3Build

} // U2

#endif // _GB2_UHMM3_BUILD_H_
