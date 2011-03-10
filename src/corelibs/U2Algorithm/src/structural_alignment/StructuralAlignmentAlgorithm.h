#ifndef _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_H_
#define _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_H_

#include <U2Core/global.h>
#include <U2Core/Matrix44.h>

namespace U2 {

class BioStruct3D;

class U2ALGORITHM_EXPORT StructuralAlignment {
public:
    double rmsd;
    Matrix44 transform;
};  // class StructuralAlignment

class U2ALGORITHM_EXPORT StructuralAlignmentAlgorithm {
public:
    virtual StructuralAlignment align(const BioStruct3D &ref, const BioStruct3D &alt, int refModel = 0, int altModel = 0) = 0;
};  // class StructuralAlignmentAlgorithm

}   // namespace U2

#endif  // #ifndef _U2_STRUCTURAL_ALIGNMENT_ALGORITHM_H_
