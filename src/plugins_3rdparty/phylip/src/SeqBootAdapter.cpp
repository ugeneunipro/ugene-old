/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

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
