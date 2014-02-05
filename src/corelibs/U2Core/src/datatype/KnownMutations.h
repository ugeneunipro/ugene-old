/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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


#ifndef _U2_KNOWN_MUTATIONS_H_
#define _U2_KNOWN_MUTATIONS_H_

#include <U2Core/U2Type.h>

namespace U2 {

/** 
    Object to store tracks of known mutations by chromosome
    chromosomes indexed from 1
    chrX = 23
    chrY = 24
*/
class KnownMutationsTrack : public U2Object {
public:
    KnownMutationsTrack():chrNumber(0) {}
    KnownMutationsTrack(const U2DataId& id, const QString& dbId, qint64 version) : U2Object(id, dbId, version){}
    
    /** Chromosome number */
    int chrNumber;

    // implement U2Object
    virtual U2DataType getType() { return U2Type::KnownMutationsTrackType; }
};

/** 
    Known mutation entity
*/

enum KnownMutationGenePosition{
    GenePosition_Invalid = -1,
    //0..1 - SIFT score
    GenePosition_Intergenic = 2,
    GenePosition_Exon = 3, //no pssm score: no homology, bad translation
    GenePosition_Gene = 4, //non-coding gene
    GenePosition_Intron = 5, 
    GenePosition_Splice = 6
};

class KnownMutation : public U2Entity {
public:
    KnownMutation():startPos(-1) {} 

    qint64      startPos;
    QByteArray  refData;
    QByteArray  obsData;
    QByteArray  dbSnpId;
    double       avSift;
    double       ljb_lrt;
    double       ljb_phylop;
    double       ljb_pp2;
    double       ljb_mt;
    double       genomes1000;  
    double       segmentalDuplication;  
    double       conserved;  
    double       gerpConserved;
    double       allFreq;
    double       hapmap;
    double       gerpScore;
    double       genePosition;

    bool isValid(){
        return startPos != -1;
    }

};


} //namespace

#endif //_U2_KNOWN_MUTATIONS_H_
