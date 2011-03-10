#ifndef _U2_BUILT_IN_CONSENSUS_ALGORITHMS_H_
#define _U2_BUILT_IN_CONSENSUS_ALGORITHMS_H_

#include <U2Core/global.h>

#include <QtCore/QString>

namespace U2 {

class U2ALGORITHM_EXPORT BuiltInConsensusAlgorithms {
public:
	static const QString DEFAULT_ALGO;
	static const QString CLUSTAL_ALGO;
	static const QString STRICT_ALGO;
    static const QString LEVITSKY_ALGO;
};

}//namespace

#endif
