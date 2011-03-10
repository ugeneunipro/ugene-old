#include "SeqBootAdapter.h"
#include "U2Core/global.h"
#include <U2Core/DNAAlphabet.h>

#include <QtCore/QSharedData>

#include <iostream>
#include <float.h>

namespace U2{
void SeqBoot::clearGenratedSequences(){
    generatedSeq.clear();
}
void SeqBoot::generateSequencesFromAlignment( const MAlignment& ma, const CreatePhyTreeSettings& settings ){
    if(!settings.bootstrap){
        return ;
    }

    malignment = &ma;
    int replicates = settings.replicates;
    generatedSeq.reserve(replicates);
    
    seqboot_getoptions();
    
    reps = replicates;

    spp = ma.getNumRows();
    sites = ma.getLength();
    loci = sites;
    maxalleles = 1;

    DNAAlphabetType alphabetType = ma.getAlphabet()->getType();

    seq_allocrest();
    seq_inputoptions();

    nodep_boot = matrix_char_new(spp, sites);
    for (int k=0; k<spp; k++){
        for(int j=0; j<sites; j++) {
            const MAlignmentRow& rowK = ma.getRow(k);
            nodep_boot[k][j] = rowK.chatAt(j);
        }
    }
    

}

} //namespace
