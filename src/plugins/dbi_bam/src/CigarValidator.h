#ifndef __CIGAR_VALIDATOR_H__
#define __CIGAR_VALIDATOR_H__

#include "Alignment.h"

namespace U2{
namespace BAM{

class CigarValidator {
public:
    CigarValidator(QList<Alignment::CigarOperation> cigar_);

    //fulfills the totalLength so caller can check if it conforms to read length
    void validate(int * totalLength);

private:
    bool static isClippingOperation(Alignment::CigarOperation::Operation op);

    bool static isRealOperation(Alignment::CigarOperation::Operation op);

    bool static isInDelOperation(Alignment::CigarOperation::Operation op);

    bool static isPaddingOperation(Alignment::CigarOperation::Operation op);

    QList<Alignment::CigarOperation> cigar;
};

}
}

#endif
