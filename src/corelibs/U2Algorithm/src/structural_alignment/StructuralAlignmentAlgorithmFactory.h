#ifndef _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_FACTORY_H_
#define _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_FACTORY_H_

#include <U2Core/global.h>

#include "StructuralAlignmentAlgorithm.h"

namespace U2 {

class U2ALGORITHM_EXPORT StructuralAlignmentAlgorithmFactory {
public:
    virtual StructuralAlignmentAlgorithm* create() = 0;
};

}   // namespace U2

#endif  // #ifndef _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_FACTORY_H_
