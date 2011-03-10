#include <U2Core/MAlignment.h>
#include <U2Core/PhyTree.h>
#include <U2Algorithm/CreatePhyTreeSettings.h>
#include <QVector>
#include <iostream>

#include "seqboot.h"

namespace U2{

typedef QVector<float> matrixrow;
typedef QVector<matrixrow> matrix;

class SeqBoot{
private:
    const MAlignment* malignment;
    QVector<sequence> generatedSeq;

public:
//    QVector<sequence>& getGeneratedSequences() const {return generatedSeq;}
    void clearGenratedSequences(); //to free memory
    void generateSequencesFromAlignment( const MAlignment& ma, const CreatePhyTreeSettings& settings );

    
};


}
