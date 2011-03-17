#include "seqboot.h"
#include "cons.h"

#include <U2Core/MAlignment.h>
#include <U2Core/PhyTree.h>
#include <U2Algorithm/CreatePhyTreeSettings.h>
#include <QVector>
#include <iostream>


namespace U2{

typedef QVector<float> matrixrow;
typedef QVector<matrixrow> matrix;

class SeqBoot{
private:
    const MAlignment* malignment;
    QVector<MAlignment*> generatedSeq;
    int seqLen;
    int seqRowCount;

   
public:
//    QVector<sequence>& getGeneratedSequences() const {return generatedSeq;}
    void clearGenratedSequences(); //to free memory
    void generateSequencesFromAlignment( const MAlignment& ma, const CreatePhyTreeSettings& settings );
    void generateDistMatrixes(const CreatePhyTreeSettings& settings);
    void consInit();

    void consensus();

    const MAlignment& getMSA(int pos);

    void initGenerSeq(int reps, int rowC, int seqLen);

    SeqBoot();
    ~SeqBoot();

    
};


}
