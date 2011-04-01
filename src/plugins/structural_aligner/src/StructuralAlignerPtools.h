#ifndef _U2_STRUCTURAL_ALIGNER_PTOOLS_H_
#define _U2_STRUCTURAL_ALIGNER_PTOOLS_H_

#include <U2Algorithm/StructuralAlignmentAlgorithm.h>
#include <U2Algorithm/StructuralAlignmentAlgorithmFactory.h>

namespace U2 {

class StructuralAlignerPtools : public StructuralAlignmentAlgorithm {
public:
    virtual StructuralAlignment align(const BioStruct3D &ref, const BioStruct3D &alt, int refModel = 0, int altModel = 0);
};  // class StructuralAlignerPtools

class StructuralAlignerPtoolsFactory : public StructuralAlignmentAlgorithmFactory {
public:
    virtual StructuralAlignmentAlgorithm* create() { return new StructuralAlignerPtools(); }
};  // class StructuralAlignerPtoolsFactory

}   // namespace U2

#endif  // #ifndef _U2_STRUCTURAL_ALIGNER_PTOOLS_H_
