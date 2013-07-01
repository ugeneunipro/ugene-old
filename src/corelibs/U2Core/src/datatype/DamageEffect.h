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
#ifndef _S3_DAMAGEEFFECT_H_
#define _S3_DAMAGEEFFECT_H_

#include <U2Core/U2Type.h>

namespace U2 {

#define DAMAGE_EFFECT_ATTRIBUTE_NAME "DEAttribute"
#define DAMAGE_EFFECT_LEN_ATTRIBUTE_NAME "DEAttribute_len"

#define DAMAGE_EFFECT_UNDEFINED_SCORE -1.0
/* Class for storing a value of SNP "Damage" effects of a variation in a gene
    if a score is not defined its value = DAMAGE_EFFECT_UNDEFINED_SCORE
*/

class DamageEffect : U2Entity{
public:

    DamageEffect():
    siftEffectValue             (DAMAGE_EFFECT_UNDEFINED_SCORE)
    ,avSift                     (DAMAGE_EFFECT_UNDEFINED_SCORE)
    ,ljb_lrt                    (DAMAGE_EFFECT_UNDEFINED_SCORE)
    ,ljb_phylop                 (DAMAGE_EFFECT_UNDEFINED_SCORE)
    ,ljb_pp2                    (DAMAGE_EFFECT_UNDEFINED_SCORE)
    ,ljb_mt                     (DAMAGE_EFFECT_UNDEFINED_SCORE)
    ,genomes1000                (DAMAGE_EFFECT_UNDEFINED_SCORE)
    ,segmentalDuplication       (false)
    ,conserved                  (false)
    ,gerpConcerved              (DAMAGE_EFFECT_UNDEFINED_SCORE)
    ,allFreq                    (DAMAGE_EFFECT_UNDEFINED_SCORE)
    ,hapmap                     (DAMAGE_EFFECT_UNDEFINED_SCORE)
    ,gerpScore                  (DAMAGE_EFFECT_UNDEFINED_SCORE)
    ,inDbSNP(false)
    ,affectedGeneName("")
    ,discardFilter("")
    ,coding(false)
    ,frameshift(false)
    {};

    
    U2DataId    variant;    /* related variant */
    
    QString     affectedGeneName;  /* related affected gene name */

    double      siftEffectValue;        /*SIFT(improved) effect value */
    
    double       avSift;    /*SIFT(original) effect value */
    
    double       ljb_lrt;   /*LRT  effect value */

    double       ljb_phylop;    /*PhyloP conservation scores */

    double       ljb_pp2;   /*PolyPhen  effect value */
    
    double       ljb_mt;    /*MutationTaster  effect value */
    
    double       genomes1000;   /*1000 Genome Project alleles frequencies*/

    bool         segmentalDuplication;   /*segmental duplication regions */

    bool         conserved;   /*conserved regions among 46 species*/

    double       gerpConcerved;   /*Conserved genomic regions by GERP++*/

    double       allFreq;   /*Alternative allele frequency in the NHLBI-ESP project*/

    double       hapmap;   /*Scores of haplotype map of the human genome*/

    double       gerpScore;   /*whole-exome GERP++ scores*/

    bool         inDbSNP;  /*if variation in dbSNP*/

    QString      discardFilter;  /*filter that discarded variation during the annotation process */

    bool         coding;    /* is variation in a coding exon or in a splice site*/

    bool         frameshift;    /*is variation non-synonymous or frameshift*/

    
};


#define PROMOTER_LEN 1000
/*
Effect of a variation on regulatory function of a gene.
*/

class RegulatoryEffect : U2Entity{
public:

    RegulatoryEffect()
        :fromGeneStartPos(DAMAGE_EFFECT_UNDEFINED_SCORE)
      {};

    U2DataId    variant;    /* related variant */

    U2DataId    affectedGeneId;  /* related affected gene name ()*/

    qint64      fromGeneStartPos; /* position from the start of transcription. Position 20 means -20 b.p. from the start */

};

} //namespace

#endif
